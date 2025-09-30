#pragma once

#include <Kokkos_Core.hpp>
#include <Kokkos_Random.hpp>
#include <ctime>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <string_view>

#include <charmtyles/util/AST.hpp>
#include <charmtyles/util/generator.hpp>
#include <charmtyles/util/matrix_view.hpp>
#include <charmtyles/util/sizes.hpp>

class Codegen {
private:
    std::vector<Kokkos::View<double*>> kkVecViews;
    std::map<int, int> vecToKkVecMap;
    std::size_t kkVecViewIdx = 0;
    std::stringstream kk;
    std::size_t kkTmpVar;
    std::vector<void*> kkCustomOps;
    std::size_t kkCustomOpIdx = 0;
    std::map<uint64_t, void*> kernel_cache;
    std::vector<Kokkos::View<double*>> vec_map;

    long long getVecIdx(size_t node_id) {
        long long vecIdx = 0;
        if(vecToKkVecMap.find(node_id) == vecToKkVecMap.end()) {
            kkVecViews.push_back(vec_map[node_id]);
            vecToKkVecMap[node_id] = kkVecViewIdx;
            return kkVecViewIdx++;
        } else {
            return vecToKkVecMap[node_id];
        }
    }

    uint64_t kernel_hash(std::string_view data) {
        const uint64_t FNV_OFFSET = 0xcbf29ce484222325ULL;
        const uint64_t FNV_PRIME  = 0x100000001b3ULL;
        uint64_t hash = FNV_OFFSET;
        for (unsigned char c : data) {
            hash ^= static_cast<uint64_t>(c);
            hash *= FNV_PRIME;
        }
        return hash;
    }

    std::string to_string(uint64_t const hash) {
        std::ostringstream oss;
        oss << std::hex << std::setw(16) << std::setfill('0') << hash;
        return oss.str();
    }

    void* getFuncPtr(ct::unary_operator* op) {
        void** vtable = *reinterpret_cast<void***>(op);
        void* fun_ptr = vtable[5];
        using RawFun = double(*)(double);
        RawFun rf = reinterpret_cast<RawFun>(fun_ptr);
        return reinterpret_cast<void*>(rf);
    }

    void* getFuncPtr(ct::binary_operator* op) {
        void** vtable = *reinterpret_cast<void***>(op);
        void* fun_ptr = vtable[5];
        using RawFun = double(*)(double, double);
        RawFun rf = reinterpret_cast<RawFun>(fun_ptr);
        return reinterpret_cast<void*>(rf);
    }

    void* compile() {
        std::string kernel_ops(kk.str());
        uint64_t hash = kernel_hash(kernel_ops);
        if(kernel_cache.find(hash) != kernel_cache.end()) {
            void* functor = kernel_cache[hash];
            return functor;
        }
        std::string file_name("kernel-" + to_string(hash) + ".cc");
        std::string lib_name ("libkernel-" + to_string(hash) + ".so");
        std::string kernel(R"(
    #include <Kokkos_Core.hpp>

    struct ASTFunctor {
        std::vector<Kokkos::View<double*>> vec_map;
        std::vector<void*> custom_ops;

        KOKKOS_INLINE_FUNCTION ASTFunctor(std::vector<Kokkos::View<double*>> _vec_map, std::vector<void*> _custom_ops)
            : vec_map(_vec_map), custom_ops(_custom_ops) {}

        KOKKOS_INLINE_FUNCTION
        void operator()(const int i) const {
    )" + kernel_ops + R"(
        }
    };

    extern "C" void run_kernel(std::vector<Kokkos::View<double*>> vec_map, std::vector<void*> custom_ops, std::size_t vec_dim) {
        ASTFunctor kernel(vec_map, custom_ops);
        Kokkos::parallel_for("debug_label", Kokkos::RangePolicy<>(0, vec_dim), kernel);
    }
    )");

        std::fstream ofs(file_name, std::ios::out);
        ofs << kernel;
        ofs.close();
        system(std::string("g++ -O3 -march=native -std=c++20 -I$PWD/_deps/kokkos-src/tpls/mdspan/include "
            "-I$PWD/_deps/kokkos-src/core/src -I$PWD/_deps/kokkos-build -shared "
            "-fPIC -o " + lib_name + " " + file_name + " -L$PWD/_deps/kokkos-build/core/src "
            "-lkokkoscore").c_str());

        void* handle = dlopen(std::string("./" + lib_name).c_str(), RTLD_NOW);
        void* functor = dlsym(handle, "run_kernel");
        kernel_cache[hash] = functor;
        return functor;
    }

#define BINOP_CODEGEN(op)                                                      \
    {                                                                          \
        long long leftid = codegen_ast(instruction, node.left_);               \
        long long rightid = codegen_ast(instruction, node.right_);             \
        kkTmpVar++;                                                            \
        kk << "auto tmp" << kkTmpVar << " = ";                                 \
        if (leftid < 0)                                                        \
        {                                                                      \
            kk << "tmp" << -leftid;                                            \
        }                                                                      \
        else                                                                   \
        {                                                                      \
            kk << "vec_map[" << leftid << "](i)";                              \
        }                                                                      \
        kk << " " << op << " ";                                                \
        if (rightid < 0)                                                       \
        {                                                                      \
            kk << "tmp" << -rightid;                                           \
        }                                                                      \
        else                                                                   \
        {                                                                      \
            kk << "vec_map[" << rightid << "](i)";                             \
        }                                                                      \
        kk << ";\n";                                                           \
    }                                                                          \
    return -static_cast<long long>(kkTmpVar);

    long long codegen_ast(
        std::vector<ct::vec_impl::vec_node> const& instruction,
        std::size_t curr_idx)
    {
        const ct::vec_impl::vec_node& node = instruction[curr_idx];

        switch (node.operation_)
        {
        case ct::util::Operation::noop:
            return getVecIdx(node.name_);
        case ct::util::Operation::add:         BINOP_CODEGEN("+")
        case ct::util::Operation::sub:         BINOP_CODEGEN("-")
        case ct::util::Operation::divide:      BINOP_CODEGEN("/")
        case ct::util::Operation::multiply:    BINOP_CODEGEN("*")
        case ct::util::Operation::eq:          BINOP_CODEGEN("==")
        case ct::util::Operation::neq:         BINOP_CODEGEN("!=")
        case ct::util::Operation::geq:         BINOP_CODEGEN(">=")
        case ct::util::Operation::leq:         BINOP_CODEGEN("<=")
        case ct::util::Operation::greater:     BINOP_CODEGEN(">")
        case ct::util::Operation::lesser:      BINOP_CODEGEN("<")
        case ct::util::Operation::logical_and: BINOP_CODEGEN("&&")
        case ct::util::Operation::logical_or:  BINOP_CODEGEN("||")
        case ct::util::Operation::logical_not: {
            long long leftid = codegen_ast(instruction, node.left_);
            kkTmpVar++;
            kk << "auto tmp" << kkTmpVar << " = !";
            if (leftid < 0) {
                kk << "tmp" << -leftid;
            } else {
                kk << "vec_map[" << leftid << "](i)";
            }
            kk << ";\n";
        } return -static_cast<long long>(kkTmpVar);
        case ct::util::Operation::unary_expr: {
            long long leftid = codegen_ast(instruction, node.left_);
            kkTmpVar++;
            kkCustomOps.push_back(getFuncPtr(node.unary_expr_.get()));
            kk << "auto tmp" << kkTmpVar << " = ";
            kk << "((double(*)(double))custom_ops[" << kkCustomOpIdx << "])(";
            if (leftid < 0) {
                kk << "tmp" << -leftid;
            } else {
                kk << "vec_map[" << leftid << "](i)";
            }
            kk << ");\n";
            kkCustomOpIdx++;
        } return -static_cast<long long>(kkTmpVar);
        case ct::util::Operation::binary_expr: {
            long long leftid = codegen_ast(instruction, node.left_);
            long long rightid = codegen_ast(instruction, node.right_);
            kkTmpVar++;
            kkCustomOps.push_back(getFuncPtr(node.binary_expr_.get()));
            kk << "auto tmp" << kkTmpVar << " = ";
            kk << "((double(*)(double,double))custom_ops[" << kkCustomOpIdx
               << "])(";
            if (leftid < 0) {
                kk << "tmp" << -leftid;
            } else {
                kk << "vec_map[" << leftid << "](i)";
            }
            kk << ", ";
            if (rightid < 0) {
                kk << "tmp" << -rightid;
            } else {
                kk << "vec_map[" << rightid << "](i)";
            }
            kk << ");\n";
            kkCustomOpIdx++;
        } return -static_cast<long long>(kkTmpVar);
        case ct::util::Operation::broadcast: {
            kkTmpVar++;
            kk << "auto tmp" << kkTmpVar << " = " << node.value_ << ";\n";
        } return -static_cast<long long>(kkTmpVar);
        case ct::util::Operation::where: {
            long long terid = codegen_ast(instruction, node.ter_);
            kkTmpVar++;
            kk << "double tmp" << kkTmpVar << ";\n";
            kk << "if (";
            if (terid < 0) {
                kk << "tmp" << -terid;
            } else {
                kk << "vec_map[" << terid << "]";
            }
            kk << "(i)) {\n";
            long long leftid = codegen_ast(instruction, node.left_);
            kk << "tmp" << kkTmpVar << " = ";
            if (leftid < 0) {
                kk << "tmp" << -leftid;
            } else {
                kk << "vec_map[" << leftid << "]";
            }
            kk << "(i);\n";
            kk << "} else {\n";
            long long rightid = codegen_ast(instruction, node.right_);
            kk << "tmp" << kkTmpVar << " = ";
            if (rightid < 0) {
                kk << "tmp" << -rightid;
            } else {
                kk << "vec_map[" << rightid << "]";
            }
            kk << "(i);\n";
            kk << "}\n";
        } return -static_cast<long long>(kkTmpVar);
        default:
            CmiAbort("Operation not implemented");
        }
        return 0.;
    }

public:
    void reset() {
        kk.str("");
        kkTmpVar = 0;
        kkCustomOpIdx = 0;
        kkCustomOps.clear();
        kkVecViews.clear();
        vecToKkVecMap.clear();
        kkVecViewIdx = 0;
    }

    void execute(std::size_t vec_dim) {
        void* functor = compile();
        ((void (*)(std::vector<Kokkos::View<double*>>, std::vector<void*>, std::size_t)) functor)(kkVecViews, kkCustomOps, vec_dim);
    }

    void generate_kernel(std::vector<ct::vec_impl::vec_node> const& instruction, std::vector<Kokkos::View<double*>> const& _vec_map) {
        this->vec_map = _vec_map;

        const size_t node_id = instruction[0].name_;

        long long resid = codegen_ast(instruction, 0);
        kk << "vec_map[" << getVecIdx(node_id) << "](i) ";
        if(instruction[0].operation_ == ct::util::Operation::inplace_add) {
            kk << "+=";
        } else if (instruction[0].operation_ == ct::util::Operation::inplace_sub) {
            kk << "-=";
        } else if (instruction[0].operation_ == ct::util::Operation::inplace_divide) {
            kk << "/=";
        } else {
            kk << "=";
        }
        kk << " tmp" << -resid << ";\n";
    }
};

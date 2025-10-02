#pragma once

#include <ctime>
#include <dlfcn.h>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string_view>
#include <tuple>

#include <charmtyles/util/AST.hpp>
#include <charmtyles/util/generator.hpp>
#include <charmtyles/util/sizes.hpp>

class Codegen
{
private:
    // a vector that stores the node_id of the vector which we get from vec_map in execute
    std::vector<size_t> kkVecViewsOrder;
    // a map from node_id -> kkVecViews Index
    std::map<int, int> vecToKkVecMap;
    std::size_t kkVecViewIdx = 0;
    // a stream to store the generated kernel
    std::stringstream kk;
    std::size_t kkTmpVar;
    // a vector that stores the index(int ast) of the node that uses some custom binary/unary ops defined by the user.
    // the second parameter is a flag that indicates if this is a unary or binary op.
    std::vector<std::pair<size_t, bool>> kkCustomOpsOrder;
    // std::vector<uintptr_t> kkCustomOps;
    std::size_t kkCustomOpIdx = 0;
    // a map from kernel hash -> Kokkos functor
    std::map<uint64_t, bool> kernel_cache;

    long long getVecIdx(size_t node_id)
    {
        long long vecIdx = 0;
        if (vecToKkVecMap.find(node_id) == vecToKkVecMap.end())
        {
            kkVecViewsOrder.emplace_back(node_id);
            vecToKkVecMap[node_id] = kkVecViewIdx;
            return kkVecViewIdx++;
        }
        else
        {
            return vecToKkVecMap[node_id];
        }
    }

    uint64_t kernel_hash(std::string_view data)
    {
        const uint64_t FNV_OFFSET = 0xcbf29ce484222325ULL;
        const uint64_t FNV_PRIME = 0x100000001b3ULL;
        uint64_t hash = FNV_OFFSET;
        for (unsigned char c : data)
        {
            hash ^= static_cast<uint64_t>(c);
            hash *= FNV_PRIME;
        }
        return hash;
    }

    std::string to_string(uint64_t const hash)
    {
        std::ostringstream oss;
        oss << std::hex << std::setw(16) << std::setfill('0') << hash;
        return oss.str();
    }

    static inline void* getFuncPtr(ct::unary_operator* op)
    {
        void** vtable = *reinterpret_cast<void***>(op);
        void* fun_ptr = vtable[5];
        using RawFun = double (*)(void*, double);
        RawFun rf = reinterpret_cast<RawFun>(fun_ptr);
        return reinterpret_cast<void*>(rf);
    }

    static inline void* getFuncPtr(ct::binary_operator* op)
    {
        void** vtable = *reinterpret_cast<void***>(op);
        void* fun_ptr = vtable[5];
        using RawFun = double (*)(void*, double, double);
        RawFun rf = reinterpret_cast<RawFun>(fun_ptr);
        return reinterpret_cast<void*>(rf);
    }

    uint64_t compile()
    {
        std::string kernel_ops(kk.str());
        uint64_t hash = kernel_hash(kernel_ops);
        if (kernel_cache.find(hash) != kernel_cache.end())
            return hash;
        std::string file_name("kernel-" + to_string(hash) + ".cc");
        std::string lib_name("libkernel-" + to_string(hash) + ".so");
        std::string kernel(R"(
    #include <Kokkos_Core.hpp>

    struct ASTFunctor {
        Kokkos::View<Kokkos::View<double*>*> vec_map;
        std::vector<void*> custom_ops;
        std::vector<void*> custom_ops_this;

        KOKKOS_INLINE_FUNCTION ASTFunctor(Kokkos::View<Kokkos::View<double*>*> _vec_map, std::vector<void*> _custom_ops, std::vector<void*> _custom_ops_this)
            : vec_map(_vec_map), custom_ops(_custom_ops), custom_ops_this(_custom_ops_this) {}

        KOKKOS_INLINE_FUNCTION
        void operator()(const int i) const {
    )" + kernel_ops +
            R"(
        }
    };

    extern "C" void run_kernel(Kokkos::View<Kokkos::View<double*>*> vec_map, std::vector<void*> custom_ops, std::vector<void*> custom_ops_this, std::size_t vec_dim) {
        ASTFunctor kernel(vec_map, custom_ops, custom_ops_this);
        Kokkos::parallel_for("debug_label", Kokkos::RangePolicy<>(0, vec_dim), kernel);
    }
    )");

        std::fstream ofs(file_name, std::ios::out);
        ofs << kernel;
        ofs.close();
#ifdef GPU_BACKEND
        system(std::string(std::string(KOKKOS_DIR) +
            "/bin/nvcc_wrapper -O3 -march=native -std=c++20 -I" +
            std::string(KOKKOS_DIR) +
            "/include "
            "-fPIC -shared -o " +
            lib_name + " " + file_name + " -L" + std::string(KOKKOS_DIR) +
            "/lib64 "
            "-lkokkoscore -L" +
            std::string(CUDA_DIR) + " -lcuda -lcudart --extended-lambda")
                .c_str());
#else
        system(std::string("g++ -O3 -march=native -std=c++20 -I" +
            std::string(KOKKOS_DIR) + "/include -shared -fPIC -o " + lib_name +
            " " + file_name + " -L" + std::string(KOKKOS_DIR) +
            "/lib -lkokkoscore")
                .c_str());
#endif
        kernel_cache[hash] = true;
        return hash;
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
        case ct::util::Operation::add:
            BINOP_CODEGEN("+")
        case ct::util::Operation::sub:
            BINOP_CODEGEN("-")
        case ct::util::Operation::divide:
            BINOP_CODEGEN("/")
        case ct::util::Operation::multiply:
            BINOP_CODEGEN("*")
        case ct::util::Operation::eq:
            BINOP_CODEGEN("==")
        case ct::util::Operation::neq:
            BINOP_CODEGEN("!=")
        case ct::util::Operation::geq:
            BINOP_CODEGEN(">=")
        case ct::util::Operation::leq:
            BINOP_CODEGEN("<=")
        case ct::util::Operation::greater:
            BINOP_CODEGEN(">")
        case ct::util::Operation::lesser:
            BINOP_CODEGEN("<")
        case ct::util::Operation::logical_and:
            BINOP_CODEGEN("&&")
        case ct::util::Operation::logical_or:
            BINOP_CODEGEN("||")
        case ct::util::Operation::logical_not:
        {
            long long leftid = codegen_ast(instruction, node.left_);
            kkTmpVar++;
            kk << "auto tmp" << kkTmpVar << " = !";
            if (leftid < 0)
            {
                kk << "tmp" << -leftid;
            }
            else
            {
                kk << "vec_map[" << leftid << "](i)";
            }
            kk << ";\n";
        }
            return -static_cast<long long>(kkTmpVar);
        case ct::util::Operation::unary_expr:
        {
            long long leftid = codegen_ast(instruction, node.left_);
            kkTmpVar++;
            kkCustomOpsOrder.push_back({curr_idx, true});
            kk << "auto tmp" << kkTmpVar << " = ";
            kk << "((double(*)(void*,double))custom_ops[" << kkCustomOpIdx
               << "])(custom_ops_this[" << kkCustomOpIdx << "], ";
            if (leftid < 0)
            {
                kk << "tmp" << -leftid;
            }
            else
            {
                kk << "vec_map[" << leftid << "](i)";
            }
            kk << ");\n";
            kkCustomOpIdx++;
        }
            return -static_cast<long long>(kkTmpVar);
        case ct::util::Operation::binary_expr:
        {
            long long leftid = codegen_ast(instruction, node.left_);
            long long rightid = codegen_ast(instruction, node.right_);
            kkTmpVar++;
            kkCustomOpsOrder.push_back({curr_idx, false});
            kk << "auto tmp" << kkTmpVar << " = ";
            kk << "((double(*)(void*,double,double))custom_ops["
               << kkCustomOpIdx << "])(custom_ops_this[" << kkCustomOpIdx
               << "], ";
            if (leftid < 0)
            {
                kk << "tmp" << -leftid;
            }
            else
            {
                kk << "vec_map[" << leftid << "](i)";
            }
            kk << ", ";
            if (rightid < 0)
            {
                kk << "tmp" << -rightid;
            }
            else
            {
                kk << "vec_map[" << rightid << "](i)";
            }
            kk << ");\n";
            kkCustomOpIdx++;
        }
            return -static_cast<long long>(kkTmpVar);
        case ct::util::Operation::broadcast:
        {
            kkTmpVar++;
            kk << "auto tmp" << kkTmpVar << " = " << node.value_ << ";\n";
        }
            return -static_cast<long long>(kkTmpVar);
        case ct::util::Operation::where:
        {
            long long terid = codegen_ast(instruction, node.ter_);
            kkTmpVar++;
            kk << "double tmp" << kkTmpVar << ";\n";
            kk << "if (";
            if (terid < 0)
            {
                kk << "tmp" << -terid;
            }
            else
            {
                kk << "vec_map[" << terid << "]";
            }
            kk << "(i)) {\n";
            long long leftid = codegen_ast(instruction, node.left_);
            kk << "tmp" << kkTmpVar << " = ";
            if (leftid < 0)
            {
                kk << "tmp" << -leftid;
            }
            else
            {
                kk << "vec_map[" << leftid << "]";
            }
            kk << "(i);\n";
            kk << "} else {\n";
            long long rightid = codegen_ast(instruction, node.right_);
            kk << "tmp" << kkTmpVar << " = ";
            if (rightid < 0)
            {
                kk << "tmp" << -rightid;
            }
            else
            {
                kk << "vec_map[" << rightid << "]";
            }
            kk << "(i);\n";
            kk << "}\n";
        }
            return -static_cast<long long>(kkTmpVar);
        default:
            CmiAbort("Operation not implemented");
        }
        return 0.;
    }

public:
    void reset()
    {
        kk.str("");
        kkTmpVar = 0;
        kkCustomOpIdx = 0;
        kkCustomOpsOrder.clear();
        kkVecViewsOrder.clear();
        vecToKkVecMap.clear();
        kkVecViewIdx = 0;
    }

    using kernelInfo = ct::vec_impl::vec_node::kernelInfo;
    using kernelType = void (*)(Kokkos::View<Kokkos::View<double*>*>,
        std::vector<void*>, std::vector<void*>, std::size_t);

    static void execute(kernelInfo const& kernel, size_t vec_dim,
        std::vector<Kokkos::View<double*>> const& vec_map,
        std::vector<ct::vec_impl::vec_node> const& instruction)
    {
        Kokkos::View<Kokkos::View<double*>*> kkVecViews(
            "kkVecViews", std::get<1>(kernel).size());
        auto kkVecViews_h = Kokkos::create_mirror_view(kkVecViews);
        for (int i = 0; i < std::get<1>(kernel).size(); i++)
        {
            kkVecViews_h(i) = vec_map[std::get<1>(kernel)[i]];
        }
        Kokkos::deep_copy(kkVecViews, kkVecViews_h);
        std::vector<void*> kkCustomOps;
        std::vector<void*> kkCustomOpsThis;

        for (auto it : std::get<2>(kernel))
            if (it.second)
            {
                kkCustomOps.emplace_back(
                    getFuncPtr(instruction[it.first].unary_expr_.get()));
                kkCustomOpsThis.emplace_back(
                    (void*) instruction[it.first].unary_expr_.get());
            }
            else
            {
                kkCustomOps.emplace_back(
                    getFuncPtr(instruction[it.first].binary_expr_.get()));
                kkCustomOpsThis.emplace_back(
                    (void*) instruction[it.first].binary_expr_.get());
            }

        void* functor =
            kokkosMgmt.ckLocalBranch()->getHandle(std::get<0>(kernel));
        ((kernelType) functor)(std::move(kkVecViews), std::move(kkCustomOps),
            std::move(kkCustomOpsThis), vec_dim);
    }

    kernelInfo generate_kernel(
        std::vector<ct::vec_impl::vec_node> const& instruction)
    {
        const size_t node_id = instruction[0].name_;

        long long resid = codegen_ast(instruction, 0);
        kk << "vec_map[" << getVecIdx(node_id) << "](i) ";
        if (instruction[0].operation_ == ct::util::Operation::inplace_add)
        {
            kk << "+=";
        }
        else if (instruction[0].operation_ == ct::util::Operation::inplace_sub)
        {
            kk << "-=";
        }
        else if (instruction[0].operation_ ==
            ct::util::Operation::inplace_divide)
        {
            kk << "/=";
        }
        else
        {
            kk << "=";
        }
        kk << " tmp" << -resid << ";\n";

        return {
            compile(), std::move(kkVecViewsOrder), std::move(kkCustomOpsOrder)};
    }
};

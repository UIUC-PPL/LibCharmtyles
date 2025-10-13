#pragma once

#include <ctime>
#include <dlfcn.h>
#include <fstream>
#include <iomanip>
#include <set>
#include <sstream>
#include <string_view>
#include <tuple>

#include <charmtyles/util/AST.hpp>
#include <charmtyles/util/generator.hpp>
#include <charmtyles/util/sizes.hpp>

class Codegen
{
private:
    // a vector that stores node_id which is used to execute to get the corresponding views from view_map
    std::vector<size_t> kkViewsOrder;
    // a map from node_id -> kkVecViews Index
    std::map<int, int> nodeToKkViewMap;
    std::size_t kkViewIdx = 0;
    // a stream to store the generated kernel
    std::stringstream kk;
    std::size_t kkTmpVar;
    // a vector that stores the index(in ast) of the node that uses some custom binary/unary ops defined by the user.
    // the second parameter is a flag that indicates if this is a unary or binary op.
    std::vector<std::pair<size_t, bool>> kkCustomOpsOrder;
    // std::vector<uintptr_t> kkCustomOps;
    std::size_t kkCustomOpIdx = 0;
    // a map from kernel hash -> Kokkos functor
    std::map<uint64_t, bool> kernel_cache;
    // indexing scheme for the corresponding n rank view
    std::string kkViewIndxScheme{};
    // View type corresponding to the given rank
    std::string kkViewType{};
    // Function declaration to run the kokkos kernel
    std::string kkFuncDecl{};
    // RangePolicy for the corresponding to the given rank
    std::string kkRangePolicy{};
    // list of indices to be sent as input to the custom unary / binary op

    size_t extraArgCount{};

    std::set<std::string> kkCustomOpsDef{};

    long long getViewIdx(size_t node_id)
    {
        if (nodeToKkViewMap.find(node_id) == nodeToKkViewMap.end())
        {
            kkViewsOrder.emplace_back(node_id);
            nodeToKkViewMap[node_id] = kkViewIdx;
            return kkViewIdx++;
        }
        else
        {
            return nodeToKkViewMap[node_id];
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

    template <size_t dim>
    static inline void* getFuncPtr(ct::unary_operator* op)
    {
        void** vtable = *reinterpret_cast<void***>(op);
        if constexpr (dim == 1)
            return vtable[5];
        else if constexpr (dim == 2)
            return vtable[6];
    }

    template <size_t dim>
    static inline void* getFuncPtr(ct::binary_operator* op)
    {
        void** vtable = *reinterpret_cast<void***>(op);
        if constexpr (dim == 1)
            return vtable[5];
        else if constexpr (dim == 2)
            return vtable[6];
    }

    uint64_t compile()
    {
        Kokkos::Timer timer;
        timer.reset();
        std::string kernel_ops(kk.str());
        uint64_t hash = kernel_hash(kernel_ops);
        if (kernel_cache.find(hash) != kernel_cache.end())
            return hash;
        std::string file_name("kernel-" + to_string(hash) + ".cc");
        std::string lib_name("libkernel-" + to_string(hash) + ".so");
        std::string kkPreamble{};
        for (auto customOpsDef : kkCustomOpsDef)
        {
            kkPreamble += customOpsDef + "\n";
        }
        std::string kernel(R"(
        #include <Kokkos_Core.hpp>
    )" + kkPreamble +
            R"(struct ASTFunctor {
        Kokkos::View<)" +
            kkViewType + R"(*> view_map;
        Kokkos::View<double*> custom_ops_args;

        KOKKOS_INLINE_FUNCTION ASTFunctor(Kokkos::View<)" +
            kkViewType +
            R"(*> _view_map, Kokkos::View<double*> custom_ops_args_)
            : view_map(_view_map), custom_ops_args(custom_ops_args_) {}

        KOKKOS_INLINE_FUNCTION
        void operator()()" +
            kkFuncDecl + R"() const {
    )" + kernel_ops +
            R"(
        }
    };

    extern "C" void run_kernel(Kokkos::View<)" +
            kkViewType +
            R"(*> view_map, Kokkos::View<double*> custom_ops_args, std::vector<std::size_t> dims) {
        ASTFunctor kernel(view_map, custom_ops_args);
        Kokkos::parallel_for("debug_label", )" +
            kkRangePolicy + R"(, kernel);
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
            "-lkokkoscore --extended-lambda")
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
        long long leftid = codegen_ast(instruction, node.left_, dim);          \
        long long rightid = codegen_ast(instruction, node.right_, dim);        \
        kkTmpVar++;                                                            \
        kk << "auto tmp" << kkTmpVar << " = ";                                 \
        if (leftid < 0)                                                        \
        {                                                                      \
            kk << "tmp" << -leftid;                                            \
        }                                                                      \
        else                                                                   \
        {                                                                      \
            kk << "view_map[" << leftid << "](" << kkViewIndxScheme << ")";    \
        }                                                                      \
        kk << " " << op << " ";                                                \
        if (rightid < 0)                                                       \
        {                                                                      \
            kk << "tmp" << -rightid;                                           \
        }                                                                      \
        else                                                                   \
        {                                                                      \
            kk << "view_map[" << rightid << "](" << kkViewIndxScheme << ")";   \
        }                                                                      \
        kk << ";\n";                                                           \
    }                                                                          \
    return -static_cast<long long>(kkTmpVar);

    inline void genIndxScheme(const size_t dim) noexcept
    {
        for (size_t i = 0; i < dim; i++)
        {
            kkViewIndxScheme += std::string(1, (char) (97 + i));
            kkFuncDecl += "const int " + std::string(1, (char) (97 + i));
            if (i != dim - 1)
            {
                kkViewIndxScheme += ", ";
                kkFuncDecl += ", ";
            }
        }
    }

    inline void genKkViewType(const size_t dim) noexcept
    {
        kkViewType += "Kokkos::View<double";
        for (size_t i = 0; i < dim; i++)
            kkViewType += "*";
        kkViewType += ">";
    }

    inline void genkkRangePolicy(const size_t dim) noexcept
    {
        if (dim == 1)
        {
            kkRangePolicy += "Kokkos::RangePolicy<>(0, dims[0])";
        }
        else
        {
            kkRangePolicy += "Kokkos::MDRangePolicy<Kokkos::Rank<" +
                std::to_string(dim) + ">>({";
            for (size_t i = 0; i < dim; i++)
            {
                kkRangePolicy += "0";
                if (i != dim - 1)
                    kkRangePolicy += ",";
            }
            kkRangePolicy += "}, {";
            for (size_t i = 0; i < dim; i++)
            {
                kkRangePolicy += "dims[" + std::to_string(i) + "]";
                if (i != dim - 1)
                    kkRangePolicy += ",";
            }
            kkRangePolicy += "})";
        }
    }

    template <typename T>
    long long codegen_ast(std::vector<T> const& instruction,
        std::size_t curr_idx, std::size_t dim)
    {
        const T& node = instruction[curr_idx];

        switch (node.operation_)
        {
        case ct::util::Operation::noop:
            return getViewIdx(node.name_);
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
            long long leftid = codegen_ast(instruction, node.left_, dim);
            kkTmpVar++;
            kk << "auto tmp" << kkTmpVar << " = !";
            if (leftid < 0)
            {
                kk << "tmp" << -leftid;
            }
            else
            {
                kk << "view_map[" << leftid << "](" << kkViewIndxScheme << ")";
            }
            kk << ";\n";
        }
            return -static_cast<long long>(kkTmpVar);
        case ct::util::Operation::unary_expr:
        {
            long long leftid = codegen_ast(instruction, node.left_, dim);
            kkTmpVar++;
            kkCustomOpsOrder.push_back({curr_idx, true});
            kk << "auto tmp" << kkTmpVar << " = ";

            std::string signature;
            if (dim == 1)
                signature = node.unary_expr_->get_vec_signature();
            else if (dim == 2)
                signature = node.unary_expr_->get_mat_signature();

            kkCustomOpsDef.insert("KOKKOS_INLINE_FUNCTION double " +
                node.unary_expr_->get_name() + signature);
            kk << node.unary_expr_->get_name() << "(" << kkViewIndxScheme
               << ", ";

            if (leftid < 0)
            {
                kk << "tmp" << -leftid;
            }
            else
            {
                kk << "view_map[" << leftid << "](" << kkViewIndxScheme << ")";
            }

            size_t argc = node.unary_expr_->get_extra_params().size();
            if (argc) kk << ", ";
            for (int i = 0; i < argc; i++)
            {
                kk << "custom_ops_args[" << extraArgCount << "]";
                if (i < argc - 1)
                    kk << ",";
                extraArgCount++;
            }
            kk << ");\n";
            kkCustomOpIdx++;
        }
            return -static_cast<long long>(kkTmpVar);
        case ct::util::Operation::binary_expr:
        {
            long long leftid = codegen_ast(instruction, node.left_, dim);
            long long rightid = codegen_ast(instruction, node.right_, dim);
            kkTmpVar++;
            kkCustomOpsOrder.push_back({curr_idx, false});
            std::string signature;
            if (dim == 1)
                signature = node.binary_expr_->get_vec_signature();
            else if (dim == 2)
                signature = node.binary_expr_->get_mat_signature();

            auto hash = kernel_hash(signature);
            kkCustomOpsDef.insert("KOKKOS_INLINE_FUNCTION double " +
                node.binary_expr_->get_name() + signature);
            kk << node.binary_expr_->get_name() << "(" << kkViewIndxScheme
               << ", ";

            if (leftid < 0)
            {
                kk << "tmp" << -leftid;
            }
            else
            {
                kk << "view_map[" << leftid << "](" << kkViewIndxScheme << ")";
            }
            kk << ", ";
            if (rightid < 0)
            {
                kk << "tmp" << -rightid;
            }
            else
            {
                kk << "view_map[" << rightid << "](" << kkViewIndxScheme << ")";
            }
            size_t argc = node.binary_expr_->get_extra_params().size();
            if (argc) kk << ", ";
            for (int i = 0; i < argc; i++)
            {
                kk << "custom_ops_args[" << extraArgCount << "]";
                if (i < argc - 1)
                    kk << ",";
                extraArgCount++;
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
            long long terid = codegen_ast(instruction, node.ter_, dim);
            kkTmpVar++;
            size_t kkResIndx = kkTmpVar;
            kk << "double tmp" << kkResIndx << ";\n";
            kk << "if (";
            if (terid < 0)
            {
                kk << "tmp" << -terid;
            }
            else
            {
                kk << "view_map[" << terid << "](" << kkViewIndxScheme << ")";
            }
            kk << ") {\n";
            long long leftid = codegen_ast(instruction, node.left_, dim);
            kk << "tmp" << kkResIndx << " = ";
            if (leftid < 0)
            {
                kk << "tmp" << -leftid;
            }
            else
            {
                kk << "view_map[" << leftid << "](" << kkViewIndxScheme << ")";
            }
            kk << ";\n";
            kk << "} else {\n";
            long long rightid = codegen_ast(instruction, node.right_, dim);
            kk << "tmp" << kkResIndx << " = ";
            if (rightid < 0)
            {
                kk << "tmp" << -rightid;
            }
            else
            {
                kk << "view_map[" << rightid << "](" << kkViewIndxScheme << ")";
            }
            kk << ";\n";
            kk << "}\n";
            return -static_cast<long long>(kkResIndx);
        }
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
        kkViewsOrder.clear();
        nodeToKkViewMap.clear();
        kkViewIdx = 0;
        kkViewIndxScheme.clear();
        kkViewType.clear();
        kkFuncDecl.clear();
        kkRangePolicy.clear();
        kkCustomOpsDef.clear();
        extraArgCount = 0;
    }

    template <typename viewType, typename nodeType, size_t dim>
    static void execute(ct::util::kernelInfo const& kernel,
        std::vector<std::size_t> dims, std::vector<viewType> const& view_map,
        std::vector<nodeType> const& instruction)
    {
        using kernelType = void (*)(Kokkos::View<viewType*>,
            Kokkos::View<double*>, std::vector<std::size_t>);

        Kokkos::View<viewType*> kkVecViews(
            "kkViews", std::get<1>(kernel).size());
        auto kkVecViews_h = Kokkos::create_mirror_view(kkVecViews);
        for (int i = 0; i < std::get<1>(kernel).size(); i++)
            kkVecViews_h(i) = view_map[std::get<1>(kernel)[i]];
        Kokkos::deep_copy(kkVecViews, kkVecViews_h);

        std::vector<double> kkCustomOpsArgs;
        for (auto it : std::get<2>(kernel))
        {
            if (it.second)
            {
                auto extra_params =
                    instruction[it.first].unary_expr_->get_extra_params();
                if (extra_params.size() == 0)
                    continue;
                kkCustomOpsArgs.insert(kkCustomOpsArgs.end(),
                    extra_params.begin(), extra_params.end());
            }
            else
            {
                auto extra_params =
                    instruction[it.first].binary_expr_->get_extra_params();
                if (extra_params.size() == 0)
                    continue;
                kkCustomOpsArgs.insert(kkCustomOpsArgs.end(),
                    extra_params.begin(), extra_params.end());
            }
        }

        Kokkos::View<double*> kkCustomOpsArgs_d(
            "kkCustomOpsArgs_d", kkCustomOpsArgs.size());
        auto kkCustomOpsArgs_h = Kokkos::create_mirror_view(kkCustomOpsArgs_d);
        for (int i = 0; i < kkCustomOpsArgs.size(); i++)
        {
            kkCustomOpsArgs_h(i) = kkCustomOpsArgs[i];
        }
        Kokkos::deep_copy(kkCustomOpsArgs_d, kkCustomOpsArgs_h);

        void* functor =
            kokkosMgmt.ckLocalBranch()->getHandle(std::get<0>(kernel));
        ((kernelType) functor)(std::move(kkVecViews),
            std::move(kkCustomOpsArgs_d), std::move(dims));
    }

    template <typename T, size_t dim>
    ct::util::kernelInfo generate_kernel(std::vector<T> const& instruction)
    {
        const size_t node_id = instruction[0].name_;
        genIndxScheme(dim);
        genKkViewType(dim);
        genkkRangePolicy(dim);

        long long resid = codegen_ast(instruction, 0, dim);
        kk << "view_map[" << getViewIdx(node_id) << "](" << kkViewIndxScheme
           << ") ";
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
            compile(), std::move(kkViewsOrder), std::move(kkCustomOpsOrder)};
    }
};

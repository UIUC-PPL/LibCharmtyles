#pragma once

#include <ctime>
#include <dlfcn.h>
#include <fstream>
#include <iomanip>
#include <set>
#include <sstream>
#include <string_view>
#include <tuple>
#include "hapi_nvtx.h"

#include <charmtyles/util/AST.hpp>
#include <charmtyles/util/generator.hpp>
#include <charmtyles/util/sizes.hpp>

using ExecSpace = Kokkos::DefaultExecutionSpace;
using double_view_1d_um = Kokkos::View<double*, Kokkos::MemoryTraits<Kokkos::Unmanaged>>;
using double_view_1d_um_host = double_view_1d_um::host_mirror_type;

template <typename ViewType>
struct Unmanagedof {
    using type = Kokkos::View<
        typename ViewType::data_type,
        typename ViewType::array_layout,
        typename ViewType::device_type,
        Kokkos::MemoryTraits<Kokkos::Unmanaged>
    >;
};

class Codegen
{
private:
    // a vector that stores node_id which is used to get the corresponding views from view_map
    std::vector<size_t> kkViewsOrder;
    // a map from node_id -> kkVecViews Index
    std::map<int, int> nodeToKkViewMap;
    std::size_t kkViewIdx = 0;
    // a stream to store the generated kernel
    std::stringstream kk;
    std::size_t kkTmpVar;
    // a vector that stores:
    // 1 -> index into the merged region of instructions
    // 2 -> the index(in ast) of the node that uses some custom binary/unary ops defined by the user.
    // 3 -> flag that indicates if this is a unary or binary op.
    std::vector<std::tuple<size_t, size_t, bool>> kkCustomOpsOrder;
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
    // list of indices to be sent as input to the custom unary / binary ops
    std::size_t kkCustomOpArgIdx{};
    // definition for the custom unops/binops that will be called in the kernel
    std::set<std::string> kkCustomOpsDef{};
    // list of scalars used by the generated kernel
    std::vector<double> kkScalarVals{};
    std::size_t kkScalarValIdx{};
    // offset into a region of multiple fused ASTs
    size_t kkRegionOffset = 0;

    std::string kkPreamble{};

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
        for (auto customOpsDef : kkCustomOpsDef)
        {
            kkPreamble += customOpsDef + "\n";
        }
        std::string kernel(R"(
        #include <Kokkos_Core.hpp>

        using ExecSpace = Kokkos::DefaultExecutionSpace;
    )" + kkPreamble +
            R"(struct ASTFunctor {
        )" +
            kkViewType + R"(* view_map;
        double* custom_ops_args;
        double* scalar_vals;

        KOKKOS_INLINE_FUNCTION ASTFunctor()" +
            kkViewType +
            R"(* _view_map, double* custom_ops_args_, double* scalar_vals_)
            : view_map(_view_map), custom_ops_args(custom_ops_args_), scalar_vals(scalar_vals_) {}

        KOKKOS_INLINE_FUNCTION
        void operator()()" +
            kkFuncDecl + R"() const {
    )" + kernel_ops +
            R"(
        }
    };

    extern "C" void run_kernel(ExecSpace& exec_space,)" +
            kkViewType +
            R"(* view_map, double* custom_ops_args, double* scalar_vals, std::vector<std::size_t> dims) {
        ASTFunctor kernel(view_map, custom_ops_args, scalar_vals);
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
            "/lib64 -lkokkoscore")
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
        kkViewType += ", Kokkos::MemoryTraits<Kokkos::Unmanaged>>";
    }

    inline void genkkRangePolicy(const size_t dim) noexcept
    {
        if (dim == 1)
        {
            kkRangePolicy += "RangePolicy(exec_space, 0, dims[0])";
        }
        else
        {
            kkRangePolicy += "MDRangePolicy(exec_space, {";
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

    inline void getkkPreamble(const size_t dim) noexcept
    {
        kkPreamble = R"(using RangePolicy = Kokkos::RangePolicy<ExecSpace>;
        using MDRangePolicy = Kokkos::MDRangePolicy<Kokkos::Rank<)"+std::to_string(dim)+R"(>, ExecSpace>;)" + kkPreamble;
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
            kkCustomOpsOrder.push_back({kkRegionOffset, curr_idx, true});
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
            if (argc)
                kk << ", ";
            for (int i = 0; i < argc; i++)
            {
                kk << "custom_ops_args[" << kkCustomOpArgIdx << "]";
                if (i < argc - 1)
                    kk << ",";
                kkCustomOpArgIdx++;
            }
            kk << ");\n";
        }
            return -static_cast<long long>(kkTmpVar);
        case ct::util::Operation::binary_expr:
        {
            long long leftid = codegen_ast(instruction, node.left_, dim);
            long long rightid = codegen_ast(instruction, node.right_, dim);
            kkTmpVar++;
            kkCustomOpsOrder.push_back({kkRegionOffset, curr_idx, false});
            kk << "auto tmp" << kkTmpVar << " = ";

            std::string signature;
            if (dim == 1)
                signature = node.binary_expr_->get_vec_signature();
            else if (dim == 2)
                signature = node.binary_expr_->get_mat_signature();

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
            if (argc)
                kk << ", ";
            for (int i = 0; i < argc; i++)
            {
                kk << "custom_ops_args[" << kkCustomOpArgIdx << "]";
                if (i < argc - 1)
                    kk << ",";
                kkCustomOpArgIdx++;
            }
            kk << ");\n";
        }
            return -static_cast<long long>(kkTmpVar);
        case ct::util::Operation::broadcast:
        {
            kkTmpVar++;
            kk << "auto tmp" << kkTmpVar << " = scalar_vals["
               << kkScalarValIdx++ << "];\n";
            kkScalarVals.emplace_back(node.value_);
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
        kkCustomOpsOrder.clear();
        kkViewsOrder.clear();
        nodeToKkViewMap.clear();
        kkViewIdx = 0;
        kkViewIndxScheme.clear();
        kkViewType.clear();
        kkFuncDecl.clear();
        kkRangePolicy.clear();
        kkCustomOpsDef.clear();
        kkCustomOpArgIdx = 0;
        kkScalarVals.clear();
        kkScalarValIdx = 0;
        kkRegionOffset = 0;
        kkPreamble.clear();
    }

    template <typename T, size_t dim>
    ct::util::kernelInfo generate_kernel(
        std::vector<std::vector<T>> const& instructions)
    {
        genIndxScheme(dim);
        genKkViewType(dim);
        genkkRangePolicy(dim);
        getkkPreamble(dim);
        
        for (auto instruction : instructions)
        {
            const size_t node_id = instruction[0].name_;
            long long resid = codegen_ast(instruction, 0, dim);
            kk << "view_map[" << getViewIdx(node_id) << "](" << kkViewIndxScheme
               << ") ";
            if (instruction[0].operation_ == ct::util::Operation::inplace_add)
            {
                kk << "+=";
            }
            else if (instruction[0].operation_ ==
                ct::util::Operation::inplace_sub)
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
            ++kkRegionOffset;
        }

        return {compile(), std::move(kkViewsOrder), std::move(kkCustomOpsOrder),
            std::move(kkScalarVals)};
    }
};

class codegen_exec {
    private:
    void* kkVecViews {};
    void* kkVecViews_h {};
    std::size_t kkVecViews_size {};//size in Bytes

    double* kkCustomOpsArgs {};
    double* kkCustomOpsArgs_h {};
    std::size_t kkCustomOpsArgs_size {};//size in Bytes

    double* kkScalarVals {};
    double* kkScalarVals_h {};
    std::size_t kkScalarVals_size {};//size in Bytes

    void resize_1d_buffers(void*& gpu_buff, void*& cpu_buff, std::size_t new_size, ExecSpace& exec_space)
    {
        if(gpu_buff!=nullptr)
            cudaFreeAsync(gpu_buff, exec_space.cuda_stream());   
        cudaMallocAsync((void**)&gpu_buff, new_size, exec_space.cuda_stream());
        if(cpu_buff!=nullptr)
            cudaFreeHost(cpu_buff);
        cudaMallocHost((void**)&cpu_buff, new_size);
    }
    public:
        template <typename viewType, typename nodeType, size_t dim>
    void execute(ExecSpace& exec_space, ct::util::kernelInfo const& kernel,
        std::vector<std::size_t> dims, std::vector<viewType> const& view_map,
        std::vector<std::vector<nodeType>> const& region)
    {
        // std::ostringstream os;
        // os << "codegen_exec::execute::begin ";
        // NVTXTracer(os.str(), NVTXColor::Turquoise);

        using viewType_um = typename Unmanagedof<viewType>::type;
        using kernelType =
            void (*)(ExecSpace&, viewType_um*, double*, double*, std::vector<std::size_t>);

        // Arrays used in the kernel
        std::size_t required_size = std::get<1>(kernel).size()*sizeof(viewType_um);
        if(kkVecViews_size < required_size)
        {
            resize_1d_buffers(kkVecViews, kkVecViews_h, (std::size_t)1.2*required_size, exec_space);
            kkVecViews_size = (std::size_t)1.2*required_size;
        }
        auto kkVecViews_um = (viewType_um*)kkVecViews;
        auto kkVecView_h_um = (viewType_um*)kkVecViews_h;
        for (int i = 0; i < std::get<1>(kernel).size(); i++)
            kkVecView_h_um[i] = viewType_um(view_map[std::get<1>(kernel)[i]]);

        // Kokkos::deep_copy(exec_space, kkVecViews, kkVecViews_h);
        cudaMemcpyAsync(kkVecViews, kkVecViews_h,  std::get<1>(kernel).size()*sizeof(viewType_um), cudaMemcpyHostToDevice, exec_space.cuda_stream());

        // Arguments to the custom operations (unop/binop) used in the kernel
        std::vector<double> kkCustomOpsArgs_v;
        for (const auto& it : std::get<2>(kernel))
        {
            if (std::get<2>(it))
            {
                auto extra_params = region[std::get<0>(it)][std::get<1>(it)]
                                        .unary_expr_->get_extra_params();
                if (extra_params.size() == 0)
                    continue;
                kkCustomOpsArgs_v.insert(kkCustomOpsArgs_v.end(),
                    extra_params.begin(), extra_params.end());
            }
            else
            {
                auto extra_params = region[std::get<0>(it)][std::get<1>(it)]
                                        .binary_expr_->get_extra_params();
                if (extra_params.size() == 0)
                    continue;
                kkCustomOpsArgs_v.insert(kkCustomOpsArgs_v.end(),
                    extra_params.begin(), extra_params.end());
            }
        }

        required_size = kkCustomOpsArgs_v.size()*sizeof(double);
        if(kkCustomOpsArgs_size < required_size)
        {
            void* gpu_tmp = (void*)this->kkCustomOpsArgs;
            void* cpu_tmp = (void*)this->kkCustomOpsArgs_h;
            resize_1d_buffers(gpu_tmp, cpu_tmp, 1.2*required_size, exec_space);
            this->kkCustomOpsArgs = (double*)gpu_tmp;
            this->kkCustomOpsArgs_h = (double*)cpu_tmp;
            kkCustomOpsArgs_size = 1.2*required_size;
        }
        for (int i = 0; i < kkCustomOpsArgs_v.size(); i++)
            kkCustomOpsArgs_h[i] = kkCustomOpsArgs_v[i];
        cudaMemcpyAsync(kkCustomOpsArgs, kkCustomOpsArgs_h, kkCustomOpsArgs_v.size()*sizeof(double), cudaMemcpyHostToDevice, exec_space.cuda_stream());

        required_size = std::get<3>(kernel).size()*sizeof(double);
        if(kkScalarVals_size < required_size)
        {
            void* gpu_tmp = (void*)this->kkScalarVals;
            void* cpu_tmp = (void*)this->kkScalarVals_h;
            resize_1d_buffers(gpu_tmp, cpu_tmp, 1.2*required_size, exec_space);
            this->kkScalarVals = (double*)gpu_tmp;
            this->kkScalarVals_h = (double*)cpu_tmp;
            kkScalarVals_size = 1.2*required_size;
        }
        for (int i = 0; i < std::get<3>(kernel).size(); i++)
            kkScalarVals_h[i] = std::get<3>(kernel)[i];
        cudaMemcpyAsync(kkScalarVals, kkScalarVals_h, std::get<3>(kernel).size()*sizeof(double), cudaMemcpyHostToDevice, exec_space.cuda_stream());

        void* functor =
            kokkosMgmt.ckLocalBranch()->getHandle(std::get<0>(kernel));
        ((kernelType) functor)(exec_space, kkVecViews_um,
            kkCustomOpsArgs, kkScalarVals,
            std::move(dims));

        // std::ostringstream os_;
        // os << "codegen_exec::execute::end ";
        // NVTXTracer(os.str(), NVTXColor::Turquoise);
    }
};

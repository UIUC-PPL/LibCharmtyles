#pragma once

#include <Kokkos_Core.hpp>
#include <Kokkos_Random.hpp>
#include <algorithm>
#include <iomanip>
#include <sstream>

class CProxy_vector_impl;
class CProxy_matrix_impl;
class CProxy_scalar_impl;
class CProxy_get_partial_vec_future;
class CProxy_KokkosGroup;

#include <charmtyles/backend/libcharmtyles.decl.h>

class KokkosGroup : public CBase_KokkosGroup
{
private:
    std::map<uint64_t, void*> kernelHandles;
    std::string to_string(uint64_t const hash)
    {
        std::ostringstream oss;
        oss << std::hex << std::setw(16) << std::setfill('0') << hash;
        return oss.str();
    }

public:
    KokkosGroup()
    {
        Kokkos::initialize();
    }

    void finalize()
    {
        // Kokkos::finalize();
    }

    void dkload(uint64_t hash)
    {
        if (kernelHandles.find(hash) != kernelHandles.end())
            return;
        std::string lib_name("libkernel-" + to_string(hash) + ".so");
        void* handle = dlopen(std::string("./" + lib_name).c_str(), RTLD_NOW);
        void* functor = dlsym(handle, "run_kernel");
        kernelHandles[hash] = functor;
    }

    void* getHandle(uint64_t hash)
    {
        return kernelHandles[hash];
    }
};

CProxy_KokkosGroup kokkosMgmt;

#include "codegen.hpp"

/* readonly */ CProxy_scalar_impl scalar_impl_proxy;

class set_future : public CBase_set_future
{
public:
    set_future(ck::future<bool> is_done_, int count)
      : is_done(is_done_)
      , total(count)
      , counter(0)
    {
    }

    void pup(PUP::er& p)
    {
        p | is_done;
        p | total;
        p | counter;
    }

    void mark_complete()
    {
        if (++counter == total)
        {
            is_done.set(true);
            counter = 0;
        }
    }

    void evaluate_bool(bool result)
    {
        is_done.set(result);
    }

private:
    ck::future<bool> is_done;
    int total;
    int counter;
};

class get_vec_future : public CBase_get_vec_future
{
public:
    get_vec_future(ck::future<std::vector<double>> output_, size_t len_)
      : output(output_)
      , len(len_)
    {
    }

    void pup(PUP::er& p)
    {
        p | output;
        p | len;
    }

    void construct_vector(CkReductionMsg* msg)
    {
        std::vector<double> out(len, 0.);
        CkReduction::setElement* current =
            (CkReduction::setElement*) msg->getData();
        while (current != NULL)
        {
            double* result = (double*) &current->data;
            size_t index = (size_t) result[0];
            size_t len = current->dataSize / sizeof(double);
            for (size_t i = 1; i < len; i++)
            {
                out[index + i - 1] = result[i];
            }
            current = current->next();
        }
        output.set(out);
    }

private:
    ck::future<std::vector<double>> output;
    size_t len;
};

class get_mat_future : public CBase_get_mat_future
{
public:
    get_mat_future(ck::future<std::vector<std::vector<double>>> output_,
        size_t rows_, size_t cols_)
      : output(output_)
      , rows(rows_)
      , cols(cols_)
    {
    }

    void pup(PUP::er& p)
    {
        p | output;
        p | rows;
        p | cols;
    }

    void construct_matrix(CkReductionMsg* msg)
    {
        std::vector<std::vector<double>> out(
            rows, std::vector<double>(cols, 0.0));
        CkReduction::setElement* current =
            (CkReduction::setElement*) msg->getData();
        while (current != NULL)
        {
            double* result = (double*) &current->data;
            size_t row_index = (size_t) result[0];
            size_t col_index = (size_t) result[1];
            size_t row_size = (size_t) result[2];
            size_t col_size = (current->dataSize - (3 * sizeof(double))) /
                (sizeof(double) * row_size);
            for (size_t i = 0; i < row_size; i++)
            {
                for (size_t j = 0; j < col_size; j++)
                    out[row_index + i][col_index + j] =
                        result[3 + i * col_size + j];
            }
            current = current->next();
        }
        output.set(out);
    }

private:
    ck::future<std::vector<std::vector<double>>> output;
    size_t rows;
    size_t cols;
};

class get_partial_vec_future : public CBase_get_partial_vec_future
{
public:
    get_partial_vec_future(ck::future<std::vector<double>> output_, size_t k_)
      : output(output_)
      , k(k_)
    {
    }

    void pup(PUP::er& p)
    {
        p | output;
        p | k;
    }

    void construct_partial_vector(CkReductionMsg* msg)
    {
        std::vector<double> out(k, 0.0);

        CkReduction::setElement* current =
            (CkReduction::setElement*) msg->getData();
        while (current != NULL)
        {
            double* result = (double*) &current->data;
            size_t len = current->dataSize / sizeof(double);

            if (len > 0)
            {
                int count = (int) result[0];
                // [count, sample_index1, value1, sample_index2, value2, ...]
                for (int i = 0; i < count && (1 + 2 * i + 1) < len; i++)
                {
                    size_t sample_index = (size_t) result[1 + 2 * i];
                    double value = result[1 + 2 * i + 1];

                    if (sample_index < k)
                    {
                        out[sample_index] = value;
                    }
                }
            }
            current = current->next();
        }

        output.set(out);
    }

private:
    ck::future<std::vector<double>> output;
    size_t k;
};

class scalar_impl : public CBase_scalar_impl
{
public:
    scalar_impl_SDAG_CODE;

    scalar_impl()
      : SDAG_INDEX(0)
    {
        thisProxy.main_kernel();
    }

    std::vector<double> scal_map;

    int SDAG_INDEX;
};

#define CHECK_IF_EXIST_ELSE_ADD_VECTOR(node)                                   \
    if (node.name_ == vec_map.size())                                          \
    {                                                                          \
        std::size_t vec_dim = get_vec_dim(node.vec_len_);                      \
                                                                               \
        Kokkos::View<double*> vec(                                             \
            "vec" + std::to_string(node.name_), vec_dim);                      \
        vec_map.emplace_back(vec);                                             \
    }

class vector_impl : public CBase_vector_impl
{
public:
    void update_partitions(
        std::vector<std::vector<ct::vec_impl::vec_node>> const& instr_list)
    {
        for (size_t i = 0; i < instr_list.size();)
        {
            std::vector<std::vector<ct::vec_impl::vec_node>> region =
                ct::util::carveRegion<ct::vec_impl::vec_node>(instr_list, i);

            if (!region.empty())
            {
                for (const auto& instr : region)
                    CHECK_IF_EXIST_ELSE_ADD_VECTOR(instr[0]);
                execute_instruction(region);
                i += region.size();
            }
            else
            {
                execute_instruction({instr_list[i]});
                ++i;
            }
        }
    }

    // Helper method for generator initialization - must be public for CUDA lambdas
    Kokkos::View<double*> generator_init_impl(
        std::size_t vec_dim, std::shared_ptr<ct::generator> gen_ptr)
    {
        Kokkos::View<double*> gen_vec("Label", vec_dim);

        for (int dimX = 0; dimX != vec_dim; ++dimX)
        {
            gen_vec(dimX) =
                gen_ptr->generate(thisIndex * vec_block_size + dimX);
        }

        return gen_vec;
    }

    // Helper method for vector dot product - must be public for CUDA lambdas
    double vector_dot_impl(int lhs_id, int rhs_id)
    {
        Kokkos::View<double*> lhs = vec_map[lhs_id];
        Kokkos::View<double*> rhs = vec_map[rhs_id];

        double result = 0.0;
        Kokkos::parallel_reduce(
            lhs.size(),
            KOKKOS_LAMBDA(const int i, double& local_sum) {
                local_sum += lhs(i) * rhs(i);
            },
            result);

        return result;
    }

    void execute_instruction(
        std::vector<std::vector<ct::vec_impl::vec_node>> const& region)
    {
        const std::vector<ct::vec_impl::vec_node>& instruction = region[0];
        const ct::vec_impl::vec_node& node = instruction[0];
        std::size_t node_id = node.name_;

        switch (node.operation_)
        {
        case ct::util::Operation::init_random:
        {
            CkAssert((vec_map.size() == node_id) &&
                "A vector is initialized before a dependent vector "
                "initialization.");

            std::size_t vec_dim = get_vec_dim(node.vec_len_);
            Kokkos::View<double*> vec("vec" + std::to_string(node_id), vec_dim);
            vec_map.emplace_back(vec);
            unsigned int seed =
                static_cast<unsigned int>(time(nullptr)) + node_id;
            Kokkos::Random_XorShift64_Pool<> rand_pool(seed);

            Kokkos::parallel_for(
                "init_random_" + std::to_string(node_id),
                vec_map[node_id].size(), KOKKOS_LAMBDA(int i) {
                    auto gen = rand_pool.get_state();
                    double r = gen.drand();
                    vec_map[node_id](i) = r;
                    rand_pool.free_state(gen);
                });
        }
            return;
        case ct::util::Operation::init_value:
        {
            CkAssert((vec_map.size() == node_id) &&
                "A vector is initialized before a dependent vector "
                "initialization.");

            std::size_t vec_dim = get_vec_dim(node.vec_len_);

            // TODO: Do Random Initialization here
            Kokkos::View<double*> vec("vec" + std::to_string(node_id), vec_dim);
            Kokkos::deep_copy(vec, node.value_);
            vec_map.emplace_back(vec);
        }
            return;
        case ct::util::Operation::copy:
        {
            std::size_t copy_id = node.copy_id_;

            if (node_id == vec_map.size())
                vec_map.emplace_back(
                    Kokkos::View<double*>("FIXME", vec_map[copy_id].size()));

            auto dest = vec_map[node_id];
            auto src = vec_map[copy_id];

            Kokkos::parallel_for(
                "copy_" + std::to_string(copy_id) + "_" +
                    std::to_string(node_id),
                dest.size(), KOKKOS_LAMBDA(int i) { dest(i) = src(i); });
        }
            return;
        case ct::util::Operation::add:
        case ct::util::Operation::sub:
        case ct::util::Operation::multiply:
        case ct::util::Operation::divide:
        case ct::util::Operation::geq:
        case ct::util::Operation::leq:
        case ct::util::Operation::greater:
        case ct::util::Operation::lesser:
        case ct::util::Operation::eq:
        case ct::util::Operation::neq:
        case ct::util::Operation::logical_and:
        case ct::util::Operation::logical_or:
        case ct::util::Operation::logical_not:
        case ct::util::Operation::unary_expr:
        case ct::util::Operation::binary_expr:
        case ct::util::Operation::where:
        {
            CHECK_IF_EXIST_ELSE_ADD_VECTOR(node);
            Codegen::execute<Kokkos::View<double*>, ct::vec_impl::vec_node, 1>(
                instruction[0].kernel, {vec_map[node_id].size()}, vec_map,
                region);
        }
            return;
        case ct::util::Operation::inplace_add:
        {
            CHECK_IF_EXIST_ELSE_ADD_VECTOR(node);
            std::size_t copy_id = node.copy_id_;
            if (copy_id == -1)
            {
                Codegen::execute<Kokkos::View<double*>, ct::vec_impl::vec_node,
                    1>(node.kernel, {vec_map[node_id].size()}, vec_map, region);
            }
            else
            {
                auto dest = vec_map[node_id];
                auto src = vec_map[copy_id];

                Kokkos::parallel_for(
                    "copy_" + std::to_string(copy_id) + "_" +
                        std::to_string(node_id),
                    dest.size(), KOKKOS_LAMBDA(int i) { dest(i) += src(i); });
            }
        }
            return;
        case ct::util::Operation::inplace_sub:
        {
            CHECK_IF_EXIST_ELSE_ADD_VECTOR(node);
            std::size_t copy_id = node.copy_id_;
            if (copy_id == -1)
            {
                Codegen::execute<Kokkos::View<double*>, ct::vec_impl::vec_node,
                    1>(node.kernel, {vec_map[node_id].size()}, vec_map, region);
            }
            else
            {
                auto dest = vec_map[node_id];
                auto src = vec_map[copy_id];

                Kokkos::parallel_for(
                    "copy_" + std::to_string(copy_id) + "_" +
                        std::to_string(node_id),
                    dest.size(), KOKKOS_LAMBDA(int i) { dest(i) -= src(i); });
            }
        }
            return;
        case ct::util::Operation::inplace_divide:
        {
            CHECK_IF_EXIST_ELSE_ADD_VECTOR(node);
            std::size_t copy_id = node.copy_id_;
            if (copy_id == -1)
            {
                Codegen::execute<Kokkos::View<double*>, ct::vec_impl::vec_node,
                    1>(node.kernel, {vec_map[node_id].size()}, vec_map, region);
            }
            else
            {
                auto dest = vec_map[node_id];
                auto src = vec_map[copy_id];

                Kokkos::parallel_for(
                    "copy_" + std::to_string(copy_id) + "_" +
                        std::to_string(node_id),
                    dest.size(), KOKKOS_LAMBDA(int i) { dest(i) /= src(i); });
            }
        }
            return;
        case ct::util::Operation::custom_expr:
        {
            CHECK_IF_EXIST_ELSE_ADD_VECTOR(node);

            const ct::vec_impl::vec_node& node = instruction[0];
            auto a_host = Kokkos::create_mirror_view_and_copy(
                Kokkos::HostSpace(), vec_map[node_id]);
            const std::size_t n = a_host.extent(0);
            auto a = std::vector<double>(a_host.data(), a_host.data() + n);

            auto b_host = Kokkos::create_mirror_view_and_copy(
                Kokkos::HostSpace(), vec_map[instruction[node.left_].name_]);
            auto b = std::vector<double>(b_host.data(), b_host.data() + n);

            node.custom_expr_->operator()(n, a, b);

            Kokkos::View<double*> a_new("vec" + std::to_string(node_id), n);
            Kokkos::View<double*> b_new(
                "vec" + std::to_string(instruction[node.left_].name_), n);

            auto a_new_host = Kokkos::create_mirror_view(a_new);
            auto b_new_host = Kokkos::create_mirror_view(b_new);

            for (auto i = 0; i < n; i++)
            {
                a_new_host(i) = a[i];
                b_new_host(i) = b[i];
            }
            Kokkos::deep_copy(a_new, a_new_host);
            Kokkos::deep_copy(b_new, b_new_host);
            vec_map[node_id] = a_new;
            vec_map[instruction[node.left_].name_] = b_new;
        }
            return;
        default:
            CmiAbort("Operation not implemented");
        }
    }

public:
    vector_impl_SDAG_CODE;

    vector_impl(int num_chares_, int vec_block_size_)
      : num_chares(num_chares_)
      , SDAG_INDEX(0)
      , vec_block_size(vec_block_size_)
    {
        vec_map.reserve(1000);

        thisProxy[thisIndex].main_kernel();
    }

private:
    std::size_t get_vec_dim(std::size_t vec_len)
    {
        if (vec_len % vec_block_size == 0)
            return vec_block_size;

        if (thisIndex != num_chares - 1)
            return vec_block_size;

        return vec_len % vec_block_size;
    }

    void print_instructions(
        std::vector<std::vector<ct::vec_impl::vec_node>> const& instr_list)
    {
        ckout << "Printing Instructions:" << endl;

        for (int num_instr = 0; num_instr != instr_list.size(); ++num_instr)
        {
            ckout << "Instruction " << num_instr << ": ";
            ct::util::parse_ast(instr_list[num_instr], 0);
            ckout << endl;
        }
    }

    int num_chares;
    std::vector<Kokkos::View<double*>> vec_map;

    int SDAG_INDEX;
    int vec_block_size;
    int dot_counter = 0;
};

#define CHECK_IF_EXIST_ELSE_ADD_MATRIX(node)                                   \
    if (node.name_ == mat_map.size())                                          \
    {                                                                          \
        std::size_t num_rows = get_mat_rows(node.mat_row_len_);                \
        std::size_t num_cols = get_mat_cols(node.mat_col_len_);                \
                                                                               \
        Kokkos::View<double**> mat(                                            \
            "mat" + std::to_string(node.name_), num_rows, num_cols);           \
        mat_map.emplace_back(mat);                                             \
    }

class matrix_impl : public CBase_matrix_impl
{
    // Helper private functions
private:
    std::size_t get_mat_rows(std::size_t row_len)
    {
        if (row_len % row_block_len == 0)
            return row_block_len;

        if (thisIndex.y != num_chares_y - 1)
            return row_block_len;

        return row_len % row_block_len;
    }

    std::size_t get_mat_cols(std::size_t col_len)
    {
        if (col_len % col_block_len == 0)
            return col_block_len;

        if (thisIndex.x != num_chares_x - 1)
            return col_block_len;

        return col_len % col_block_len;
    }

    void print_instructions(
        std::vector<std::vector<ct::mat_impl::mat_node>> const& instr_list)
    {
        ckout << "Printing Instructions:" << endl;

        for (int num_instr = 0; num_instr != instr_list.size(); ++num_instr)
        {
            ckout << "Instruction " << num_instr << ": ";
            ct::util::parse_ast(instr_list[num_instr], 0);
            ckout << endl;
        }
    }

    // Instruction related functions - must be public for CUDA lambdas
public:
    void update_partitions(
        std::vector<std::vector<ct::mat_impl::mat_node>> const& instr_list)
    {
        for (size_t i = 0; i < instr_list.size();)
        {
            std::vector<std::vector<ct::mat_impl::mat_node>> region =
                ct::util::carveRegion<ct::mat_impl::mat_node>(instr_list, i);

            if (!region.empty())
            {
                for (const auto& instr : region)
                    CHECK_IF_EXIST_ELSE_ADD_MATRIX(instr[0]);
                execute_instruction(region);
                i += region.size();
            }
            else
            {
                execute_instruction({instr_list[i]});
                ++i;
            }
        }
    }

    // Helper method for matrix-vector multiplication - must be public for CUDA lambdas
    void mat_vec_dot_impl(int mat_idx, const double* vec_in_data,
        std::size_t vec_len, Kokkos::View<double*>& local_result)
    {
        Kokkos::View<double**> mat = mat_map[mat_idx];
        std::size_t num_rows = mat.extent(0);
        std::size_t num_cols = mat.extent(1);

        CkAssert(vec_len >= num_cols &&
            "Incoming vector does not have enough entries for this matrix "
            "tile");

        std::size_t offset = 0;
        if (vec_len > num_cols)
        {
            std::size_t max_offset = vec_len - num_cols;
            offset = std::min<std::size_t>(
                static_cast<std::size_t>(thisIndex.x) * col_block_len,
                max_offset);
        }

        using HostConstVector = Kokkos::View<const double*, Kokkos::HostSpace,
            Kokkos::MemoryTraits<Kokkos::Unmanaged>>;
        HostConstVector vec_in_host(vec_in_data + offset, num_cols);

        using DeviceVector = Kokkos::View<double*,
            typename Kokkos::DefaultExecutionSpace::memory_space>;
        DeviceVector vec_in("vec_in_tile", num_cols);
        Kokkos::deep_copy(vec_in, vec_in_host);

        // Perform matrix-vector multiplication: result = mat * vec
        Kokkos::parallel_for(
            "mat_vec_dot", num_rows, KOKKOS_LAMBDA(int i) {
                double sum = 0.0;
                for (std::size_t j = 0; j < num_cols; ++j)
                {
                    sum += mat(i, j) * vec_in(j);
                }
                local_result(i) = sum;
            });
        Kokkos::fence();
    }

    // Helper method for vector-matrix multiplication - must be public for CUDA lambdas
    void vec_mat_dot_impl(int mat_idx, const double* vec_in_data,
        std::size_t vec_len, Kokkos::View<double*>& local_result)
    {
        Kokkos::View<double**> mat = mat_map[mat_idx];
        std::size_t num_rows = mat.extent(0);
        std::size_t num_cols = mat.extent(1);

        CkAssert(vec_len >= num_rows &&
            "Incoming vector does not have enough entries for this matrix "
            "tile");

        std::size_t offset = 0;
        if (vec_len > num_rows)
        {
            std::size_t max_offset = vec_len - num_rows;
            offset = std::min<std::size_t>(
                static_cast<std::size_t>(thisIndex.y) * row_block_len,
                max_offset);
        }

        using HostConstVector = Kokkos::View<const double*, Kokkos::HostSpace,
            Kokkos::MemoryTraits<Kokkos::Unmanaged>>;
        HostConstVector vec_in_host(vec_in_data + offset, num_rows);

        using DeviceVector = Kokkos::View<double*,
            typename Kokkos::DefaultExecutionSpace::memory_space>;
        DeviceVector vec_in("vec_in_tile", num_rows);
        Kokkos::deep_copy(vec_in, vec_in_host);

        // Perform vector-matrix multiplication: result = vec * mat
        Kokkos::parallel_for(
            "vec_mat_dot", num_cols, KOKKOS_LAMBDA(int j) {
                double sum = 0.0;
                for (std::size_t i = 0; i < num_rows; ++i)
                {
                    sum += vec_in(i) * mat(i, j);
                }
                local_result(j) = sum;
            });
        Kokkos::fence();
    }

    void execute_instruction(
        std::vector<std::vector<ct::mat_impl::mat_node>> const& region)
    {
        const std::vector<ct::mat_impl::mat_node>& instruction = region[0];
        ct::mat_impl::mat_node const& node = instruction[0];
        std::size_t node_id = node.name_;

        switch (node.operation_)
        {
        case ct::util::Operation::init_random:
        {
            CkAssert((mat_map.size() == node_id) &&
                "A matrix is initialized before a dependent matrix "
                "initialization.");

            std::size_t num_rows = get_mat_rows(node.mat_row_len_);
            std::size_t num_cols = get_mat_cols(node.mat_col_len_);

            Kokkos::View<double**> mat(
                "mat" + std::to_string(node_id), num_rows, num_cols);
            unsigned int seed =
                static_cast<unsigned int>(time(nullptr)) + node_id;
            Kokkos::Random_XorShift64_Pool<> rand_pool(seed);

            Kokkos::parallel_for(
                "init_random_mat_" + std::to_string(node_id),
                Kokkos::MDRangePolicy<Kokkos::Rank<2>>(
                    {0, 0}, {num_rows, num_cols}),
                KOKKOS_LAMBDA(int i, int j) {
                    auto gen = rand_pool.get_state();
                    double r = gen.drand();
                    mat(i, j) = r;
                    rand_pool.free_state(gen);
                });
            mat_map.emplace_back(mat);
        }
            return;
        case ct::util::Operation::init_value:
        {
            CkAssert((mat_map.size() == node_id) &&
                "A matrix is initialized before a dependent matrix "
                "initialization.");

            std::size_t num_rows = get_mat_rows(node.mat_row_len_);
            std::size_t num_cols = get_mat_cols(node.mat_col_len_);

            Kokkos::View<double**> mat(
                "mat" + std::to_string(node_id), num_rows, num_cols);
            Kokkos::deep_copy(mat, node.value_);
            mat_map.emplace_back(mat);
        }
            return;
        case ct::util::Operation::copy:
        {
            std::size_t copy_id = node.copy_id_;
            CHECK_IF_EXIST_ELSE_ADD_MATRIX(node);
            auto dest = mat_map[node_id];
            auto src = mat_map[copy_id];

            Kokkos::parallel_for(
                "copy_mat_" + std::to_string(copy_id) + "_" +
                    std::to_string(node_id),
                Kokkos::MDRangePolicy<Kokkos::Rank<2>>(
                    {0, 0}, {dest.extent(0), dest.extent(1)}),
                KOKKOS_LAMBDA(int i, int j) { dest(i, j) = src(i, j); });
        }
            return;
        case ct::util::Operation::add:
        case ct::util::Operation::sub:
        case ct::util::Operation::multiply:
        case ct::util::Operation::divide:
        case ct::util::Operation::geq:
        case ct::util::Operation::leq:
        case ct::util::Operation::greater:
        case ct::util::Operation::lesser:
        case ct::util::Operation::eq:
        case ct::util::Operation::neq:
        case ct::util::Operation::unary_expr:
        case ct::util::Operation::binary_expr:
        case ct::util::Operation::logical_and:
        case ct::util::Operation::logical_or:
        case ct::util::Operation::logical_not:
        case ct::util::Operation::where:
        {
            CHECK_IF_EXIST_ELSE_ADD_MATRIX(node);
            Codegen::execute<Kokkos::View<double**>, ct::mat_impl::mat_node, 2>(
                instruction[0].kernel,
                {mat_map[node_id].extent(0), mat_map[node_id].extent(1)},
                mat_map, region);
        }
            return;
        case ct::util::Operation::inplace_add:
        {
            CHECK_IF_EXIST_ELSE_ADD_MATRIX(node);
            std::size_t copy_id = node.copy_id_;
            if (copy_id == -1)
            {
                Codegen::execute<Kokkos::View<double**>, ct::mat_impl::mat_node,
                    2>(instruction[0].kernel,
                    {mat_map[node_id].extent(0), mat_map[node_id].extent(1)},
                    mat_map, region);
            }
            else
            {
                auto dest = mat_map[node_id];
                auto src = mat_map[copy_id];

                Kokkos::parallel_for(
                    "inplace_add_mat_" + std::to_string(node_id),
                    Kokkos::MDRangePolicy<Kokkos::Rank<2>>(
                        {0, 0}, {dest.extent(0), dest.extent(1)}),
                    KOKKOS_LAMBDA(int i, int j) { dest(i, j) += src(i, j); });
            }
        }
            return;
        case ct::util::Operation::inplace_sub:
        {
            CHECK_IF_EXIST_ELSE_ADD_MATRIX(node);
            std::size_t copy_id = node.copy_id_;
            if (copy_id == -1)
            {
                Codegen::execute<Kokkos::View<double**>, ct::mat_impl::mat_node,
                    2>(instruction[0].kernel,
                    {mat_map[node_id].extent(0), mat_map[node_id].extent(1)},
                    mat_map, region);
            }
            else
            {
                auto dest = mat_map[node_id];
                auto src = mat_map[copy_id];

                Kokkos::parallel_for(
                    "inplace_add_mat_" + std::to_string(node_id),
                    Kokkos::MDRangePolicy<Kokkos::Rank<2>>(
                        {0, 0}, {dest.extent(0), dest.extent(1)}),
                    KOKKOS_LAMBDA(int i, int j) { dest(i, j) -= src(i, j); });
            }
        }
            return;
        case ct::util::Operation::inplace_divide:
        {
            CHECK_IF_EXIST_ELSE_ADD_MATRIX(node);
            std::size_t copy_id = node.copy_id_;
            if (copy_id == -1)
            {
                Codegen::execute<Kokkos::View<double**>, ct::mat_impl::mat_node,
                    2>(instruction[0].kernel,
                    {mat_map[node_id].extent(0), mat_map[node_id].extent(1)},
                    mat_map, region);
            }
            else
            {
                auto dest = mat_map[node_id];
                auto src = mat_map[copy_id];

                Kokkos::parallel_for(
                    "inplace_add_mat_" + std::to_string(node_id),
                    Kokkos::MDRangePolicy<Kokkos::Rank<2>>(
                        {0, 0}, {dest.extent(0), dest.extent(1)}),
                    KOKKOS_LAMBDA(int i, int j) { dest(i, j) /= src(i, j); });
            }
        }
            return;
        case ct::util::Operation::custom_expr:
        {
            CHECK_IF_EXIST_ELSE_ADD_MATRIX(node);

            const ct::mat_impl::mat_node& node = instruction[0];
            auto a_host = Kokkos::create_mirror_view_and_copy(
                Kokkos::HostSpace(), mat_map[node_id]);
            const std::size_t rows = a_host.extent(0);
            const std::size_t cols = a_host.extent(1);

            std::vector<std::vector<double>> a;
            for (int i = 0; i < rows; i++)
            {
                a.push_back(std::vector<double>(
                    a_host.data() + i * cols, a_host.data() + i * cols + cols));
            }

            auto b_host = Kokkos::create_mirror_view_and_copy(
                Kokkos::HostSpace(), mat_map[instruction[node.left_].name_]);
            std::vector<std::vector<double>> b;
            for (int i = 0; i < rows; i++)
            {
                b.push_back(std::vector<double>(
                    b_host.data() + i * cols, b_host.data() + i * cols + cols));
            }

            node.custom_expr_->operator()(rows, cols, a, b);

            Kokkos::View<double**> a_new(
                "mat" + std::to_string(node_id), rows, cols);
            Kokkos::View<double**> b_new(
                "mat" + std::to_string(instruction[node.left_].name_), rows,
                cols);

            auto a_new_host = Kokkos::create_mirror_view(a_new);
            auto b_new_host = Kokkos::create_mirror_view(b_new);

            for (int i = 0; i < rows; i++)
            {
                for (int j = 0; j < cols; j++)
                {
                    a_new_host(i, j) = a[i][j];
                    b_new_host(i, j) = b[i][j];
                }
            }

            Kokkos::deep_copy(a_new, a_new_host);
            Kokkos::deep_copy(b_new, b_new_host);
            mat_map[node_id] = a_new;
            mat_map[instruction[node.left_].name_] = b_new;
        }
            return;
        default:
            CmiAbort("Operation not implemented");
        }
    }

public:
    matrix_impl_SDAG_CODE;

    matrix_impl(int num_chares_y_, int num_chares_x_, int row_block_len_,
        int col_block_len_)
      : num_chares_y(num_chares_y_)
      , num_chares_x(num_chares_x_)
      , row_block_len(row_block_len_)
      , col_block_len(col_block_len_)
      , SDAG_INDEX(0)
    {
        mat_map.reserve(1000);
        thisProxy(thisIndex.x, thisIndex.y).main_kernel();
    }

private:
    std::vector<Kokkos::View<double**>> mat_map;

    int num_chares_y;
    int num_chares_x;

    int row_block_len;
    int col_block_len;
    int SDAG_INDEX;
    int block;
};

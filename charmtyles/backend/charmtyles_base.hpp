#pragma once

#include <Kokkos_Core.hpp>
#include <Kokkos_Random.hpp>
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

class vector_impl : public CBase_vector_impl
{
public:
    void update_partitions(
        std::vector<std::vector<ct::vec_impl::vec_node>> const& instr_list)
    {
        for (auto const& ast : instr_list)
            execute_instruction(ast);
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

#define CHECK_IF_EXIST_ELSE_ADD_VECTOR(node_id)                                \
    if (node_id == vec_map.size())                                             \
    {                                                                          \
        vec_dim = get_vec_dim(node.vec_len_);                                  \
                                                                               \
        Kokkos::View<double*> vec("vec" + std::to_string(node_id), vec_dim);   \
        vec_map.emplace_back(vec);                                             \
    }

    void execute_instruction(
        std::vector<ct::vec_impl::vec_node> const& instruction,
        std::size_t index = 0)
    {
        ct::vec_impl::vec_node const& node = instruction[index];
        std::size_t node_id = node.name_;

        // Useful variables in switch statement
        std::size_t vec_dim{0};
        std::size_t total_size{0};
        std::size_t unrolled_size{0};
        std::size_t remainder_start{0};
        std::size_t copy_id{0};

        std::shared_ptr<ct::unary_operator> const& unary_expr =
            node.unary_expr_;

        std::shared_ptr<ct::binary_operator> const& binary_expr =
            node.binary_expr_;

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dist(0., 1.);

        switch (instruction[index].operation_)
        {
        case ct::util::Operation::init_random:
        {
            CkAssert((vec_map.size() == node_id) &&
                "A vector is initialized before a dependent vector "
                "initialization.");

            vec_dim = get_vec_dim(node.vec_len_);
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

            vec_dim = get_vec_dim(node.vec_len_);

            // TODO: Do Random Initialization here
            Kokkos::View<double*> vec("vec" + std::to_string(node_id), vec_dim);
            Kokkos::deep_copy(vec, node.value_);
            vec_map.emplace_back(vec);
        }
            return;
        case ct::util::Operation::copy:
        {
            copy_id = node.copy_id_;

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
            CHECK_IF_EXIST_ELSE_ADD_VECTOR(node_id);

            Codegen::execute<Kokkos::View<double*>, ct::vec_impl::vec_node, 1>(
                instruction[0].kernel, {vec_map[node_id].size()}, vec_map,
                instruction);
        }
            return;
        case ct::util::Operation::inplace_add:
        {
            CHECK_IF_EXIST_ELSE_ADD_VECTOR(node_id);
            copy_id = node.copy_id_;
            if (copy_id == -1)
            {
                Codegen::execute<Kokkos::View<double*>, ct::vec_impl::vec_node,
                    1>(node.kernel, {vec_map[node_id].size()}, vec_map,
                    instruction);
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
            CHECK_IF_EXIST_ELSE_ADD_VECTOR(node_id);
            copy_id = node.copy_id_;
            if (copy_id == -1)
            {
                Codegen::execute<Kokkos::View<double*>, ct::vec_impl::vec_node,
                    1>(node.kernel, {vec_map[node_id].size()}, vec_map,
                    instruction);
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
            CHECK_IF_EXIST_ELSE_ADD_VECTOR(node_id);
            copy_id = node.copy_id_;
            if (copy_id == -1)
            {
                Codegen::execute<Kokkos::View<double*>, ct::vec_impl::vec_node,
                    1>(node.kernel, {vec_map[node_id].size()}, vec_map,
                    instruction);
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
        case ct::util::Operation::axpy:
        {
            CHECK_IF_EXIST_ELSE_ADD_VECTOR(node_id);

            double alpha = node.value_;
            Kokkos::View<double*> x = vec_map[node.left_];
            Kokkos::View<double*> y = vec_map[node.right_];
            Kokkos::View<double*> res = vec_map[node.name_];

            Kokkos::parallel_for(
                "axpy", x.extent(0),
                KOKKOS_LAMBDA(const int i) { res(i) = alpha * x(i) + y(i); });
        }
            return;
        case ct::util::Operation::custom_expr:
        {
            CHECK_IF_EXIST_ELSE_ADD_VECTOR(node_id);

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
        for (auto const& ast : instr_list)
            execute_instruction(ast);
    }

    // Helper method for matrix-vector multiplication - must be public for CUDA lambdas
    void mat_vec_dot_impl(int mat_idx, double* vec_in_data,
        std::size_t vec_size, Kokkos::View<double*>& local_result)
    {
        Kokkos::View<double*> vec_in(vec_in_data, vec_size);
        Kokkos::View<double**> mat = mat_map[mat_idx];
        std::size_t num_rows = mat.extent(0);

        // Perform matrix-vector multiplication: result = mat * vec
        Kokkos::parallel_for(
            "mat_vec_dot", num_rows, KOKKOS_LAMBDA(int i) {
                double sum = 0.0;
                for (std::size_t j = 0; j < vec_size; ++j)
                {
                    sum += mat(i, j) * vec_in(j);
                }
                local_result(i) = sum;
            });
        Kokkos::fence();
    }

    // Helper method for vector-matrix multiplication - must be public for CUDA lambdas
    void vec_mat_dot_impl(int mat_idx, double* vec_in_data,
        std::size_t vec_size, Kokkos::View<double*>& local_result)
    {
        Kokkos::View<double*> vec_in(vec_in_data, vec_size);
        Kokkos::View<double**> mat = mat_map[mat_idx];
        std::size_t num_cols = mat.extent(1);

        // Perform vector-matrix multiplication: result = vec * mat
        Kokkos::parallel_for(
            "vec_mat_dot", num_cols, KOKKOS_LAMBDA(int j) {
                double sum = 0.0;
                for (std::size_t i = 0; i < vec_size; ++i)
                {
                    sum += vec_in(i) * mat(i, j);
                }
                local_result(j) = sum;
            });
        Kokkos::fence();
    }

#define CHECK_IF_EXIST_ELSE_ADD_MATRIX(node_id)                                \
    if (node_id == mat_map.size())                                             \
    {                                                                          \
        num_rows = get_mat_rows(node.mat_row_len_);                            \
        num_cols = get_mat_cols(node.mat_col_len_);                            \
                                                                               \
        Kokkos::View<double**> mat(                                            \
            "mat" + std::to_string(node_id), num_rows, num_cols);              \
        mat_map.emplace_back(mat);                                             \
    }

    void execute_instruction(
        std::vector<ct::mat_impl::mat_node> const& instruction,
        std::size_t index = 0)
    {
        ct::mat_impl::mat_node const& node = instruction[index];
        std::size_t node_id = node.name_;
        std::shared_ptr<ct::unary_operator> const& unary_expr =
            node.unary_expr_;
        std::shared_ptr<ct::binary_operator> const& binary_expr =
            node.binary_expr_;

        // Useful variables in switch statement
        std::size_t num_rows{0};
        std::size_t num_cols{0};
        std::size_t total_size{0};
        std::size_t unrolled_size{0};
        std::size_t remainder_start{0};
        std::size_t copy_id{0};

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dist(0., 1.);

        switch (node.operation_)
        {
        case ct::util::Operation::init_random:

            CkAssert((mat_map.size() == node_id) &&
                "A matrix is initialized before a dependent matrix "
                "initialization.");

            num_rows = get_mat_rows(node.mat_row_len_);
            num_cols = get_mat_cols(node.mat_col_len_);

            {
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
            CkAssert((mat_map.size() == node_id) &&
                "A matrix is initialized before a dependent matrix "
                "initialization.");

            num_rows = get_mat_rows(node.mat_row_len_);
            num_cols = get_mat_cols(node.mat_col_len_);

            {
                Kokkos::View<double**> mat(
                    "mat" + std::to_string(node_id), num_rows, num_cols);
                Kokkos::deep_copy(mat, node.value_);
                mat_map.emplace_back(mat);
            }

            return;

        case ct::util::Operation::copy:
            copy_id = node.copy_id_;
            CHECK_IF_EXIST_ELSE_ADD_MATRIX(node_id);

            {
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
            CHECK_IF_EXIST_ELSE_ADD_MATRIX(node_id);
            Codegen::execute<Kokkos::View<double**>, ct::mat_impl::mat_node, 2>(
                instruction[0].kernel,
                {mat_map[node_id].extent(0), mat_map[node_id].extent(1)},
                mat_map, instruction);
        }
            return;
        case ct::util::Operation::inplace_add:
        {
            CHECK_IF_EXIST_ELSE_ADD_MATRIX(node_id);
            copy_id = node.copy_id_;
            if (copy_id == -1)
            {
                Codegen::execute<Kokkos::View<double**>, ct::mat_impl::mat_node,
                    2>(instruction[0].kernel,
                    {mat_map[node_id].extent(0), mat_map[node_id].extent(1)},
                    mat_map, instruction);
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
            CHECK_IF_EXIST_ELSE_ADD_MATRIX(node_id);
            copy_id = node.copy_id_;
            if (copy_id == -1)
            {
                Codegen::execute<Kokkos::View<double**>, ct::mat_impl::mat_node,
                    2>(instruction[0].kernel,
                    {mat_map[node_id].extent(0), mat_map[node_id].extent(1)},
                    mat_map, instruction);
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
            CHECK_IF_EXIST_ELSE_ADD_MATRIX(node_id);
            copy_id = node.copy_id_;
            if (copy_id == -1)
            {
                Codegen::execute<Kokkos::View<double**>, ct::mat_impl::mat_node,
                    2>(instruction[0].kernel,
                    {mat_map[node_id].extent(0), mat_map[node_id].extent(1)},
                    mat_map, instruction);
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
            CHECK_IF_EXIST_ELSE_ADD_MATRIX(node_id);

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

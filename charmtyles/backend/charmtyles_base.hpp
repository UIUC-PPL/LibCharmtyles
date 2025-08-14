#pragma once

#include <charmtyles/util/AST.hpp>
#include <charmtyles/util/generator.hpp>
#include <charmtyles/util/matrix_view.hpp>
#include <charmtyles/util/sizes.hpp>

class CProxy_vector_impl;
class CProxy_matrix_impl;
class CProxy_scalar_impl;
class CProxy_get_partial_vec_future;

#include <charmtyles/backend/libcharmtyles.decl.h>

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

private:
    std::vector<double> scal_map;

    int SDAG_INDEX;
};

class vector_impl : public CBase_vector_impl
{
    // Helper private functions
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

    // Instruction related private functions
private:
    void update_partitions(
        std::vector<std::vector<ct::vec_impl::vec_node>> const& instr_list)
    {
        for (auto const& ast : instr_list)
            execute_instruction(ast);
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
        auto safe_div_vec = [&](std::size_t idx, double denom) {
            if (denom != 0.0) vec_map[node_id][idx] /= denom;
            else CmiAbort("divide by zero");
        };

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dist(0., 1.);

        switch (instruction[index].operation_)
        {
        case ct::util::Operation::init_random:

            CkAssert((vec_map.size() == node_id) &&
                "A vector is initialized before a dependent vector "
                "initialization.");

            vec_dim = get_vec_dim(node.vec_len_);

            // TODO: Do Random Initialization here
            vec_map.emplace_back(std::vector<double>(vec_dim));

            for (double& val : vec_map[node.name_])
            {
                val = dist(gen);
            }

            return;

        case ct::util::Operation::init_value:
            CkAssert((vec_map.size() == node_id) &&
                "A vector is initialized before a dependent vector "
                "initialization.");

            vec_dim = get_vec_dim(node.vec_len_);

            // TODO: Do Random Initialization here
            vec_map.emplace_back(std::vector<double>(vec_dim, node.value_));

            return;

        case ct::util::Operation::copy:
            copy_id = node.copy_id_;

            if (node_id == vec_map.size())
                vec_map.emplace_back(
                    std::vector<double>(vec_map[copy_id].size()));

            total_size = vec_map[node_id].size();
            unrolled_size = vec_map[node_id].size() / 4;
            remainder_start = unrolled_size * 4;

            for (std::size_t i = 0; i != remainder_start; i += 4)
            {
                vec_map[node_id][i] = vec_map[copy_id][i];
                vec_map[node_id][i + 1] = vec_map[copy_id][i + 1];
                vec_map[node_id][i + 2] = vec_map[copy_id][i + 2];
                vec_map[node_id][i + 3] = vec_map[copy_id][i + 3];
            }

            for (std::size_t i = remainder_start; i != total_size; ++i)
            {
                vec_map[node_id][i] = vec_map[copy_id][i];
            }

            return;

        case ct::util::Operation::add:
        case ct::util::Operation::sub:
        case ct::util::Operation::divide:

            if (node_id == vec_map.size())
            {
                vec_dim = get_vec_dim(node.vec_len_);

                vec_map.emplace_back(std::vector<double>(vec_dim));
            }

            total_size = vec_map[node_id].size();
            unrolled_size = vec_map[node_id].size() / 4;
            remainder_start = unrolled_size * 4;

            for (std::size_t i = 0; i != remainder_start; i += 4)
            {
                vec_map[node_id][i] = execute_ast_for_idx(instruction, 0, i);
                vec_map[node_id][i + 1] =
                    execute_ast_for_idx(instruction, 0, i + 1);
                vec_map[node_id][i + 2] =
                    execute_ast_for_idx(instruction, 0, i + 2);
                vec_map[node_id][i + 3] =
                    execute_ast_for_idx(instruction, 0, i + 3);
            }

            for (std::size_t i = remainder_start; i != total_size; ++i)
            {
                vec_map[node_id][i] = execute_ast_for_idx(instruction, 0, i);
            }

            return;
        case ct::util::Operation::inplace_add:
            copy_id = node.copy_id_;
            if (node_id == vec_map.size())
            {
                vec_dim = get_vec_dim(node.vec_len_);
                vec_map.emplace_back(std::vector<double>(vec_dim));
            }
            total_size = vec_map[node_id].size();
            unrolled_size = total_size / 4;
            remainder_start = unrolled_size * 4;
            
            for (std::size_t i = 0; i != remainder_start; i += 4){
                if (copy_id == static_cast<std::size_t>(-1))
                {
                    vec_map[node_id][i]     += execute_ast_for_idx(instruction, 1, i);
                    vec_map[node_id][i + 1] += execute_ast_for_idx(instruction, 1, i + 1);
                    vec_map[node_id][i + 2] += execute_ast_for_idx(instruction, 1, i + 2);
                    vec_map[node_id][i + 3] += execute_ast_for_idx(instruction, 1, i + 3);
                } else {
                    vec_map[node_id][i]     += vec_map[copy_id][i];
                    vec_map[node_id][i + 1] += vec_map[copy_id][i + 1];
                    vec_map[node_id][i + 2] += vec_map[copy_id][i + 2];
                    vec_map[node_id][i + 3] += vec_map[copy_id][i + 3];
                }
            }

            for (std::size_t i = 0; i != remainder_start; i += 4)
            {
                if (copy_id == static_cast<std::size_t>(-1))
                    vec_map[node_id][i] += execute_ast_for_idx(instruction, 1, i);
                else
                    vec_map[node_id][i] += vec_map[copy_id][i];
            }
                
            return;

        case ct::util::Operation::inplace_sub:
            copy_id = node.copy_id_;
            if (node_id == vec_map.size())
            {
                vec_dim = get_vec_dim(node.vec_len_);
                vec_map.emplace_back(std::vector<double>(vec_dim));
            }
            total_size = vec_map[node_id].size();
            unrolled_size = total_size / 4;
            remainder_start = unrolled_size * 4;
            for (std::size_t i = 0; i != remainder_start; i += 4){
                if (copy_id == static_cast<std::size_t>(-1))
                {
                    vec_map[node_id][i]     -= execute_ast_for_idx(instruction, 1, i);
                    vec_map[node_id][i + 1] -= execute_ast_for_idx(instruction, 1, i + 1);
                    vec_map[node_id][i + 2] -= execute_ast_for_idx(instruction, 1, i + 2);
                    vec_map[node_id][i + 3] -= execute_ast_for_idx(instruction, 1, i + 3);
                } else {
                    vec_map[node_id][i]     -= vec_map[copy_id][i];
                    vec_map[node_id][i + 1] -= vec_map[copy_id][i + 1];
                    vec_map[node_id][i + 2] -= vec_map[copy_id][i + 2];
                    vec_map[node_id][i + 3] -= vec_map[copy_id][i + 3];
                }
            }

            for (std::size_t i = 0; i != remainder_start; i += 4)
            {
                if (copy_id == static_cast<std::size_t>(-1))
                    vec_map[node_id][i] -= execute_ast_for_idx(instruction, 1, i);
                else
                    vec_map[node_id][i] -= vec_map[copy_id][i];
            }

        case ct::util::Operation::inplace_divide:
            copy_id = node.copy_id_;
            if (node_id == vec_map.size())
            {
                vec_dim = get_vec_dim(node.vec_len_);
                vec_map.emplace_back(std::vector<double>(vec_dim));
            }
            total_size = vec_map[node_id].size();
            unrolled_size = total_size / 4;
            remainder_start = unrolled_size * 4;
            for (std::size_t i = 0; i != remainder_start; i += 4){
                if (copy_id == static_cast<std::size_t>(-1))
                {
                    safe_div_vec(i, execute_ast_for_idx(instruction, 1, i));
                    safe_div_vec(i + 1, execute_ast_for_idx(instruction, 1, i + 1));
                    safe_div_vec(i + 2, execute_ast_for_idx(instruction, 1, i + 2));
                    safe_div_vec(i + 3, execute_ast_for_idx(instruction, 1, i + 3));
                } else {
                    safe_div_vec(i, vec_map[copy_id][i]);
                    safe_div_vec(i + 1, vec_map[copy_id][i + 1]);
                    safe_div_vec(i + 2, vec_map[copy_id][i + 2]);
                    safe_div_vec(i + 3, vec_map[copy_id][i + 3]);
                }
            }

            for (std::size_t i = 0; i != remainder_start; i += 4)
            {
                if (copy_id == static_cast<std::size_t>(-1))
                    safe_div_vec(i, execute_ast_for_idx(instruction, 1, i));
                else
                    safe_div_vec(i, vec_map[copy_id][i]);
            }
            return;

        case ct::util::Operation::unary_expr:
            if (node_id == vec_map.size())
            {
                vec_dim = get_vec_dim(node.vec_len_);

                vec_map.emplace_back(std::vector<double>(vec_dim));
            }
            total_size = vec_map[node_id].size();
            unrolled_size = vec_map[node_id].size() / 4;
            remainder_start = unrolled_size * 4;

            for (std::size_t i = 0; i != remainder_start; i += 4)
            {
                vec_map[node_id][i] = unary_expr->operator()(
                    i, vec_map[instruction[node.left_].name_][i]);
                vec_map[node_id][i + 1] = unary_expr->operator()(
                    i + 1, vec_map[instruction[node.left_].name_][i + 1]);
                vec_map[node_id][i + 2] = unary_expr->operator()(
                    i + 2, vec_map[instruction[node.left_].name_][i + 2]);
                vec_map[node_id][i + 3] = unary_expr->operator()(
                    i + 3, vec_map[instruction[node.left_].name_][i + 3]);
            }

            for (std::size_t i = remainder_start; i != total_size; ++i)
            {
                vec_map[node_id][i] = unary_expr->operator()(
                    i, vec_map[instruction[node.left_].name_][i]);
            }

            return;

        case ct::util::Operation::binary_expr:
            if (node_id == vec_map.size())
            {
                vec_dim = get_vec_dim(node.vec_len_);

                vec_map.emplace_back(std::vector<double>(vec_dim));
            }

            total_size = vec_map[node_id].size();
            unrolled_size = vec_map[node_id].size() / 4;
            remainder_start = unrolled_size * 4;

            for (std::size_t i = 0; i != remainder_start; i += 4)
            {
                vec_map[node_id][i] = binary_expr->operator()(i,
                    vec_map[instruction[node.left_].name_][i],
                    vec_map[instruction[node.right_].name_][i]);
                vec_map[node_id][i + 1] = binary_expr->operator()(i + 1,
                    vec_map[instruction[node.left_].name_][i + 1],
                    vec_map[instruction[node.right_].name_][i + 1]);
                vec_map[node_id][i + 2] = binary_expr->operator()(i + 2,
                    vec_map[instruction[node.left_].name_][i + 2],
                    vec_map[instruction[node.right_].name_][i + 2]);
                vec_map[node_id][i + 3] = binary_expr->operator()(i + 3,
                    vec_map[instruction[node.left_].name_][i + 3],
                    vec_map[instruction[node.right_].name_][i + 3]);
            }

            for (std::size_t i = remainder_start; i != total_size; ++i)
            {
                vec_map[node_id][i] = binary_expr->operator()(i,
                    vec_map[instruction[node.left_].name_][i],
                    vec_map[instruction[node.right_].name_][i]);
            }

            return;

        case ct::util::Operation::axpy:
        {
            if (node_id == vec_map.size())
            {
                vec_dim = get_vec_dim(node.vec_len_);

                vec_map.emplace_back(std::vector<double>(vec_dim));
            }

            double alpha = node.value_;
            std::vector<double>& x = vec_map[node.left_];
            std::vector<double>& y = vec_map[node.right_];
            std::vector<double>& res = vec_map[node.name_];

            Eigen::Map<Eigen::VectorXd> ex(x.data(), x.size());
            Eigen::Map<Eigen::VectorXd> ey(y.data(), y.size());
            Eigen::Map<Eigen::VectorXd> er(res.data(), res.size());

            er = alpha * ex + ey;

            return;
        }
        default:
            CmiAbort("Operation not implemented");
        }
    }

    double execute_ast_for_idx(
        std::vector<ct::vec_impl::vec_node> const& instruction,
        std::size_t curr_idx, std::size_t iter_idx)
    {
        const ct::vec_impl::vec_node& node = instruction[curr_idx];

        switch (node.operation_)
        {
        case ct::util::Operation::noop:
            return vec_map[node.name_][iter_idx];

        case ct::util::Operation::add:
            return execute_ast_for_idx(instruction, node.left_, iter_idx) +
                execute_ast_for_idx(instruction, node.right_, iter_idx);

        case ct::util::Operation::sub:
            return execute_ast_for_idx(instruction, node.left_, iter_idx) -
                execute_ast_for_idx(instruction, node.right_, iter_idx);

        case ct::util::Operation::divide:
            return execute_ast_for_idx(instruction, node.left_, iter_idx) /
                execute_ast_for_idx(instruction, node.right_, iter_idx);
        default:
            CmiAbort("Operation not implemented");
        }

        // Control should not reach here!
        return 0.;
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
    int num_chares;
    std::vector<std::vector<double>> vec_map;

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

    // Instruction related private functions
private:
    void update_partitions(
        std::vector<std::vector<ct::mat_impl::mat_node>> const& instr_list)
    {
        for (auto const& ast : instr_list)
            execute_instruction(ast);
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
        ct::util::matrix_view mat{};
        auto safe_div = [&](std::size_t I, std::size_t J, double denom) {
            if (denom != 0.0) mat_map[node_id](I, J) /= denom;
            else {
                CmiAbort("divide by zero");
            }
        };

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

            mat = ct::util::matrix_view{num_rows, num_cols};
            for (int row = 0; row != mat.rows(); ++row)
                for (int col = 0; col != mat.cols(); ++col)
                    mat(row, col) = dist(gen);

            mat_map.emplace_back(std::move(mat));

            return;

        case ct::util::Operation::init_value:
            CkAssert((mat_map.size() == node_id) &&
                "A matrix is initialized before a dependent matrix "
                "initialization.");

            num_rows = get_mat_rows(node.mat_row_len_);
            num_cols = get_mat_cols(node.mat_col_len_);

            mat = ct::util::matrix_view{num_rows, num_cols, node.value_};
            mat_map.emplace_back(std::move(mat));

            return;

        case ct::util::Operation::copy:
            copy_id = node.copy_id_;

            if (node_id == mat_map.size())
            {
                num_rows = get_mat_rows(node.mat_row_len_);
                num_cols = get_mat_cols(node.mat_col_len_);

                mat = ct::util::matrix_view{num_rows, num_cols};

                mat_map.emplace_back(std::move(mat));
            }

            total_size = mat_map[node_id].rows();
            unrolled_size = mat_map[node_id].rows() / 4;
            remainder_start = unrolled_size * 4;

            for (std::size_t i = 0; i != remainder_start; i += 4)
            {
                for (std::size_t j = 0; j != mat_map[node_id].cols(); ++j)
                {
                    mat_map[node_id](i, j) = mat_map[copy_id](i, j);
                    mat_map[node_id](i + 1, j) = mat_map[copy_id](i + 1, j);
                    mat_map[node_id](i + 2, j) = mat_map[copy_id](i + 2, j);
                    mat_map[node_id](i + 3, j) = mat_map[copy_id](i + 3, j);
                }
            }

            for (std::size_t i = remainder_start; i != total_size; ++i)
            {
                for (std::size_t j = 0; j != mat_map[node_id].cols(); ++j)
                {
                    mat_map[node_id](i, j) = mat_map[copy_id](i, j);
                }
            }

            return;

        case ct::util::Operation::add:
        case ct::util::Operation::sub:
        case ct::util::Operation::divide:
            // if the matrix is being declared here
            if (node_id == mat_map.size())
            {
                num_rows = get_mat_rows(node.mat_row_len_);
                num_cols = get_mat_cols(node.mat_col_len_);

                mat = ct::util::matrix_view{num_rows, num_cols};

                mat_map.emplace_back(std::move(mat));
            }

            total_size = mat_map[node_id].rows();
            unrolled_size = mat_map[node_id].rows() / 4;
            remainder_start = unrolled_size * 4;

            for (std::size_t i = 0; i != remainder_start; i += 4)
            {
                for (std::size_t j = 0; j != mat_map[node_id].cols(); ++j)
                {
                    mat_map[node_id](i, j) =
                        execute_ast_for_idx(instruction, 0, i, j);
                    mat_map[node_id](i + 1, j) =
                        execute_ast_for_idx(instruction, 0, i + 1, j);
                    mat_map[node_id](i + 2, j) =
                        execute_ast_for_idx(instruction, 0, i + 2, j);
                    mat_map[node_id](i + 3, j) =
                        execute_ast_for_idx(instruction, 0, i + 3, j);
                }
            }

            for (std::size_t i = remainder_start; i != total_size; ++i)
            {
                for (std::size_t j = 0; j != mat_map[node_id].cols(); ++j)
                {
                    mat_map[node_id](i, j) =
                        execute_ast_for_idx(instruction, 0, i, j);
                }
            }

            return;
        case ct::util::Operation::inplace_add:
            copy_id = node.copy_id_;
            if (node_id == mat_map.size())
            {
                num_rows = get_mat_rows(node.mat_row_len_);
                num_cols = get_mat_cols(node.mat_col_len_);

                mat = ct::util::matrix_view{num_rows, num_cols};

                mat_map.emplace_back(std::move(mat));
            }

            total_size = mat_map[node_id].rows();
            unrolled_size = mat_map[node_id].rows() / 4;
            remainder_start = unrolled_size * 4;

            for (std::size_t i = 0; i != remainder_start; i += 4)
            {
                for (std::size_t j = 0; j != mat_map[node_id].cols(); ++j)
                {
                    if(copy_id == -1) {
                        mat_map[node_id](i, j) +=
                            execute_ast_for_idx(instruction, 1, i, j);
                        mat_map[node_id](i + 1, j) +=
                            execute_ast_for_idx(instruction, 1, i + 1, j);
                        mat_map[node_id](i + 2, j) +=
                            execute_ast_for_idx(instruction, 1, i + 2, j);
                        mat_map[node_id](i + 3, j) +=
                            execute_ast_for_idx(instruction, 1, i + 3, j);
                    } else {
                        mat_map[node_id](i, j) += mat_map[copy_id](i, j);
                        mat_map[node_id](i + 1, j) += mat_map[copy_id](i + 1, j);
                        mat_map[node_id](i + 2, j) += mat_map[copy_id](i + 2, j);
                        mat_map[node_id](i + 3, j) += mat_map[copy_id](i + 3, j);
                    }
                }
            }

            for (std::size_t i = remainder_start; i != total_size; ++i)
            {
                for (std::size_t j = 0; j != mat_map[node_id].cols(); ++j)
                {
                    if(copy_id == -1)
                        mat_map[node_id](i, j) += execute_ast_for_idx(instruction, 1, i, j);
                    else
                        mat_map[node_id](i, j) += mat_map[copy_id](i, j);
                }
            }

            return;
        case ct::util::Operation::inplace_sub:
            copy_id = node.copy_id_;
            if (node_id == mat_map.size())
            {
                num_rows = get_mat_rows(node.mat_row_len_);
                num_cols = get_mat_cols(node.mat_col_len_);
                mat = ct::util::matrix_view{num_rows, num_cols};
                mat_map.emplace_back(std::move(mat));
            }

            total_size = mat_map[node_id].rows();
            unrolled_size = mat_map[node_id].rows() / 4;
            remainder_start = unrolled_size * 4;

            for (std::size_t i = 0; i != remainder_start; i += 4)
            {
                for (std::size_t j = 0; j != mat_map[node_id].cols(); ++j)
                {
                    if (copy_id == static_cast<std::size_t>(-1))
                    {
                        mat_map[node_id](i, j)     -= execute_ast_for_idx(instruction, 1, i,     j);
                        mat_map[node_id](i + 1, j) -= execute_ast_for_idx(instruction, 1, i + 1, j);
                        mat_map[node_id](i + 2, j) -= execute_ast_for_idx(instruction, 1, i + 2, j);
                        mat_map[node_id](i + 3, j) -= execute_ast_for_idx(instruction, 1, i + 3, j);
                    }
                    else
                    {
                        mat_map[node_id](i, j)     -= mat_map[copy_id](i,     j);
                        mat_map[node_id](i + 1, j) -= mat_map[copy_id](i + 1, j);
                        mat_map[node_id](i + 2, j) -= mat_map[copy_id](i + 2, j);
                        mat_map[node_id](i + 3, j) -= mat_map[copy_id](i + 3, j);
                    }
                }
            }

            for (std::size_t i = remainder_start; i != total_size; ++i)
            {
                for (std::size_t j = 0; j != mat_map[node_id].cols(); ++j)
                {
                    if (copy_id == static_cast<std::size_t>(-1))
                        mat_map[node_id](i, j) -= execute_ast_for_idx(instruction, 1, i, j);
                    else
                        mat_map[node_id](i, j) -= mat_map[copy_id](i, j);
                }
            }
            return;

        case ct::util::Operation::inplace_divide:
            copy_id = node.copy_id_;
            if (node_id == mat_map.size())
            {
                num_rows = get_mat_rows(node.mat_row_len_);
                num_cols = get_mat_cols(node.mat_col_len_);
                mat = ct::util::matrix_view{num_rows, num_cols};
                mat_map.emplace_back(std::move(mat));
            }

            total_size = mat_map[node_id].rows();
            unrolled_size = mat_map[node_id].rows() / 4;
            remainder_start = unrolled_size * 4;

            for (std::size_t i = 0; i != remainder_start; i += 4)
            {
                for (std::size_t j = 0; j != mat_map[node_id].cols(); ++j)
                {
                    if (copy_id == static_cast<std::size_t>(-1))
                    {
                        safe_div(i,     j, execute_ast_for_idx(instruction, 1, i,     j));
                        safe_div(i + 1, j, execute_ast_for_idx(instruction, 1, i + 1, j));
                        safe_div(i + 2, j, execute_ast_for_idx(instruction, 1, i + 2, j));
                        safe_div(i + 3, j, execute_ast_for_idx(instruction, 1, i + 3, j));
                    }
                    else
                    {
                        safe_div(i,     j, mat_map[copy_id](i,     j));
                        safe_div(i + 1, j, mat_map[copy_id](i + 1, j));
                        safe_div(i + 2, j, mat_map[copy_id](i + 2, j));
                        safe_div(i + 3, j, mat_map[copy_id](i + 3, j));
                    }
                }
            }

            for (std::size_t i = remainder_start; i != total_size; ++i)
            {
                for (std::size_t j = 0; j != mat_map[node_id].cols(); ++j)
                {
                    if (copy_id == static_cast<std::size_t>(-1))
                        safe_div(i, j, execute_ast_for_idx(instruction, 1, i, j));
                    else
                        safe_div(i, j, mat_map[copy_id](i, j));
                }
            }
            return;

        case ct::util::Operation::unary_expr:
            if (node_id == mat_map.size())
            {
                num_rows = get_mat_rows(node.mat_row_len_);
                num_cols = get_mat_cols(node.mat_col_len_);

                mat = ct::util::matrix_view{num_rows, num_cols};

                mat_map.emplace_back(std::move(mat));
            }
            total_size = mat_map[node_id].cols();
            unrolled_size = mat_map[node_id].cols() / 4;
            remainder_start = unrolled_size * 4;
            for (std::size_t i = 0; i != mat_map[node_id].rows(); i++)
            {
                for (std::size_t j = 0; j != remainder_start; j += 4)
                {
                    mat_map[node_id](i, j) = unary_expr->operator()(
                        i, j, mat_map[instruction[node.left_].name_](i, j));
                    mat_map[node_id](i, j + 1) =
                        unary_expr->operator()(i, j + 1,
                            mat_map[instruction[node.left_].name_](i, j + 1));
                    mat_map[node_id](i, j + 2) =
                        unary_expr->operator()(i, j + 2,
                            mat_map[instruction[node.left_].name_](i, j + 2));
                    mat_map[node_id](i, j + 3) =
                        unary_expr->operator()(i, j + 3,
                            mat_map[instruction[node.left_].name_](i, j + 3));
                }
            }
            for (std::size_t i = 0; i != mat_map[node_id].rows(); ++i)
            {
                for (std::size_t j = remainder_start; j != total_size; ++j)
                {
                    mat_map[node_id](i, j) = unary_expr->operator()(
                        i, j, mat_map[instruction[node.left_].name_](i, j));
                }
            }

            return;
        case ct::util::Operation::binary_expr:
            if (node_id == mat_map.size())
            {
                num_rows = get_mat_rows(node.mat_row_len_);
                num_cols = get_mat_cols(node.mat_col_len_);

                mat = ct::util::matrix_view{num_rows, num_cols};

                mat_map.emplace_back(std::move(mat));
            }

            total_size = mat_map[node_id].cols();
            unrolled_size = mat_map[node_id].cols() / 4;
            remainder_start = unrolled_size * 4;

            for (std::size_t i = 0; i != mat_map[node_id].rows(); i++)
            {
                for (std::size_t j = 0; j != remainder_start; j += 4)
                {
                    mat_map[node_id](i, j) = binary_expr->operator()(i, j,
                        mat_map[instruction[node.left_].name_](i, j),
                        mat_map[instruction[node.right_].name_](i, j));
                    mat_map[node_id](i, j + 1) = binary_expr->operator()(i,
                        j + 1, mat_map[instruction[node.left_].name_](i, j + 1),
                        mat_map[instruction[node.right_].name_](i, j + 1));
                    mat_map[node_id](i, j + 2) = binary_expr->operator()(i,
                        j + 2, mat_map[instruction[node.left_].name_](i, j + 2),
                        mat_map[instruction[node.right_].name_](i, j + 2));
                    mat_map[node_id](i, j + 3) = binary_expr->operator()(i,
                        j + 3, mat_map[instruction[node.left_].name_](i, j + 3),
                        mat_map[instruction[node.right_].name_](i, j + 3));
                }
            }

            for (std::size_t i = 0; i != mat_map[node_id].rows(); ++i)
            {
                for (std::size_t j = remainder_start; j != total_size; ++j)
                {
                    mat_map[node_id](i, j) = binary_expr->operator()(i, j,
                        mat_map[instruction[node.left_].name_](i, j),
                        mat_map[instruction[node.right_].name_](i, j));
                }
            }

            return;

        default:
            CmiAbort("Operation not implemented");
        }
    }

    double execute_ast_for_idx(
        std::vector<ct::mat_impl::mat_node> const& instruction,
        std::size_t curr_idx, std::size_t iter_i, std::size_t iter_j)
    {
        const ct::mat_impl::mat_node& node = instruction[curr_idx];

        switch (node.operation_)
        {
        case ct::util::Operation::noop:
            return mat_map[node.name_](iter_i, iter_j);

        case ct::util::Operation::add:
            return execute_ast_for_idx(
                       instruction, node.left_, iter_i, iter_j) +
                execute_ast_for_idx(instruction, node.right_, iter_i, iter_j);

        case ct::util::Operation::sub:
            return execute_ast_for_idx(
                       instruction, node.left_, iter_i, iter_j) -
                execute_ast_for_idx(instruction, node.right_, iter_i, iter_j);

        case ct::util::Operation::divide:
            return execute_ast_for_idx(
                       instruction, node.left_, iter_i, iter_j) /
                execute_ast_for_idx(instruction, node.right_, iter_i, iter_j);
        default:
            CmiAbort("Operation not implemented");
        }

        // Control should not reach here!
        return 0.;
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
    std::vector<ct::util::matrix_view> mat_map;

    int num_chares_y;
    int num_chares_x;

    int row_block_len;
    int col_block_len;
    int SDAG_INDEX;
    int block;
};

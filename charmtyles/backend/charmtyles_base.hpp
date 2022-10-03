#pragma once

#include <charmtyles/util/AST.hpp>
#include <charmtyles/util/sizes.hpp>

#include <charmtyles/backend/libcharmtyles.decl.h>

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
        std::size_t unrolled_size{0};
        std::size_t remainder_start{0};
        std::size_t copy_id{0};

        switch (instruction[index].operation_)
        {
        case ct::util::Operation::init_random:

            CkAssert((vec_map.size() + 1 == node_id) &&
                "A vector is initialized before a dependent vector "
                "initialization.");

            vec_dim = get_vec_dim(node.vec_len_);

            // TODO: Do Random Initialization here
            vec_map.emplace_back(std::vector<double>(vec_dim));

            return;

        case ct::util::Operation::init_value:
            CkAssert((vec_map.size() + 1 == node_id) &&
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

            unrolled_size = vec_map[node_id].size() / 4;
            for (std::size_t i = 0; i != unrolled_size; i += 4)
            {
                vec_map[node_id][i] = vec_map[copy_id][i];
                vec_map[node_id][i + 1] = vec_map[copy_id][i + 1];
                vec_map[node_id][i + 2] = vec_map[copy_id][i + 2];
                vec_map[node_id][i + 3] = vec_map[copy_id][i + 3];
            }

            remainder_start = unrolled_size * 4;
            for (std::size_t i = remainder_start; i != vec_map[node_id].size();
                 ++i)
            {
                vec_map[node_id][i] = vec_map[copy_id][i];
            }

            return;

        case ct::util::Operation::add:
        case ct::util::Operation::sub:

            if (node_id == vec_map.size())
            {
                vec_dim = get_vec_dim(node.vec_len_);

                vec_map.emplace_back(std::vector<double>(vec_dim));
            }

            unrolled_size = vec_map[node_id].size() / 4;
            for (std::size_t i = 0; i != unrolled_size; i += 4)
            {
                vec_map[node_id][i] = execute_ast_for_idx(instruction, 0, i);
            }

            remainder_start = unrolled_size * 4;
            for (std::size_t i = remainder_start; i != vec_map[node_id].size();
                 ++i)
            {
                vec_map[node_id][i] = execute_ast_for_idx(instruction, 0, i);
            }
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
};

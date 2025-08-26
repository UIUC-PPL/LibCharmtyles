#pragma once

#include <cstdint>
#include <iostream>
#include <vector>

#include "charm++.h"

#include <charmtyles/util/generator.hpp>

namespace ct {
    namespace util {

        enum class Operation : short
        {
            // Leaf Nodes
            noop = 0,
            init_random = 1,
            init_value = 2,
            init_generate = 3,
            copy = 4,

            // Middle/Head Nodes
            add = 10,
            sub = 11,
            divide = 12,
            inplace_add = 13,
            inplace_sub = 14,
            inplace_divide = 15,

            // Blas
            axpy = 20,

            // Unary operations
            unary_expr = 30,
            // Binary operations
            binary_expr = 40,
        };

        inline bool is_init_type(ct::util::Operation op)
        {
            if (op == ct::util::Operation::noop)
                return true;

            if (op == ct::util::Operation::init_value)
                return true;

            if (op == ct::util::Operation::init_random)
                return true;

            if (op == ct::util::Operation::copy)
                return true;

            return false;
        }

        template <typename ASTNode>
        void parse_ast(std::vector<ASTNode> const& instr, std::size_t index)
        {
            if (index == 0 && instr[index].operation_ == ct::util::Operation::inplace_add){
                ckout << instr[index].name_ << " += ";
                parse_ast(instr, instr[index].right_);
                return;
            }
            if (index == 0 && instr[index].operation_ == ct::util::Operation::inplace_sub){
                ckout << instr[index].name_ << " -= ";
                parse_ast(instr, instr[index].right_);
                return;
            }
            if (index == 0 && instr[index].operation_ == ct::util::Operation::inplace_divide){
                ckout << instr[index].name_ << " /= ";
                parse_ast(instr, instr[index].right_);
                return;
            }
            if (index == 0 && !is_init_type(instr[index].operation_))
                ckout << instr[index].name_ << " = ";

            switch (instr[index].operation_)
            {
            case Operation::init_random:
                ckout << instr[index].name_ << " = [RANDOM]";
                return;
            case Operation::init_value:
                ckout << instr[index].name_ << " = VALUE["
                      << instr[index].value_ << "]";
                return;
            case Operation::noop:
                ckout << instr[index].name_;
                return;
            case Operation::copy:
                ckout << instr[index].name_ << " = " << instr[index].copy_id_;
                return;
            case Operation::add:
                parse_ast(instr, instr[index].left_);
                ckout << " + ";
                parse_ast(instr, instr[index].right_);
                return;
            case Operation::sub:
                parse_ast(instr, instr[index].left_);
                ckout << " - ";
                parse_ast(instr, instr[index].right_);
                return;
            default:
                CmiAbort("Operation not implemented");
            }
        }

    }    // namespace util

    namespace vec_impl {

        struct vec_node
        {
            std::size_t name_ = -1;
            ct::util::Operation operation_;
            std::shared_ptr<ct::unary_operator> unary_expr_ =
                std::make_shared<ct::unary_operator>();
            std::shared_ptr<ct::binary_operator> binary_expr_ =
                std::make_shared<ct::binary_operator>();
            std::size_t copy_id_ = -1;
            double value_ = 0.;

            std::size_t vec_len_;

            std::size_t left_ = -1;
            std::size_t right_ = -1;

            // Only called when initializing through expression
            vec_node() = default;

            explicit vec_node(ct::util::Operation op, std::size_t size)
              : operation_(op)
              , vec_len_(size)
            {
            }

            explicit vec_node(
                std::size_t name, ct::util::Operation op, std::size_t size)
              : name_(name)
              , operation_(op)
              , vec_len_(size)
            {
            }

            explicit vec_node(std::size_t name, ct::util::Operation op,
                std::shared_ptr<ct::unary_operator> unary_expr,
                std::size_t vec_len)
              : name_(name)
              , operation_(op)
              , unary_expr_(unary_expr)
              , vec_len_(vec_len)
            {
            }

            explicit vec_node(std::size_t name, ct::util::Operation op,
                std::shared_ptr<ct::binary_operator> binary_expr,
                std::size_t vec_len)
              : name_(name)
              , operation_(op)
              , binary_expr_(binary_expr)
              , vec_len_(vec_len)
            {
            }

            explicit vec_node(
                std::size_t name, ct::util::Operation op, vec_node const& other)
              : name_(name)
              , operation_(op)
              , copy_id_(other.name_)
              , vec_len_(other.vec_len_)
            {
            }

            explicit vec_node(std::size_t name, ct::util::Operation op,
                double value, std::size_t size)
              : name_(name)
              , operation_(op)
              , value_(value)
              , vec_len_(size)
            {
            }

            explicit vec_node(std::size_t name, ct::util::Operation op,
                double alpha, std::size_t size, std::size_t x, std::size_t y)
              : name_(name)
              , operation_(op)
              , value_(alpha)
              , vec_len_(size)
              , left_(x)
              , right_(y)
            {
            }

            void pup(PUP::er& p)
            {
                p | name_;
                p | operation_;
                p | copy_id_;
                p | unary_expr_;
                p | binary_expr_;
                p | value_;
                p | vec_len_;
                p | left_;
                p | right_;
            }
        };

    }    // namespace vec_impl

    namespace mat_impl {

        struct mat_node
        {
            std::size_t name_ = -1;
            ct::util::Operation operation_;
            std::shared_ptr<ct::unary_operator> unary_expr_ =
                std::make_shared<ct::unary_operator>();
            std::shared_ptr<ct::binary_operator> binary_expr_ =
                std::make_shared<ct::binary_operator>();
            std::size_t copy_id_ = -1;
            double value_ = 0.;

            std::size_t mat_row_len_;
            std::size_t mat_col_len_;

            std::size_t left_ = -1;
            std::size_t right_ = -1;

            // Only called when initializing through expression
            mat_node() = default;

            explicit mat_node(
                ct::util::Operation op, std::size_t rows, std::size_t cols)
              : operation_(op)
              , mat_row_len_(rows)
              , mat_col_len_(cols)
            {
            }

            explicit mat_node(std::size_t matrix_id, ct::util::Operation op,
                std::size_t rows, std::size_t cols)
              : name_(matrix_id)
              , operation_(op)
              , mat_row_len_(rows)
              , mat_col_len_(cols)
            {
            }

            explicit mat_node(std::size_t matrix_id, ct::util::Operation op,
                std::shared_ptr<ct::unary_operator> unary_expr,
                std::size_t rows, std::size_t cols)
              : name_(matrix_id)
              , operation_(op)
              , unary_expr_(unary_expr)
              , mat_row_len_(rows)
              , mat_col_len_(cols)
            {
            }

            explicit mat_node(std::size_t matrix_id, ct::util::Operation op,
                std::shared_ptr<ct::binary_operator> binary_expr,
                std::size_t rows, std::size_t cols)
              : name_(matrix_id)
              , operation_(op)
              , binary_expr_(binary_expr)
              , mat_row_len_(rows)
              , mat_col_len_(cols)
            {
            }

            explicit mat_node(std::size_t matrix_id, ct::util::Operation op,
                double value, std::size_t rows, std::size_t cols)
              : name_(matrix_id)
              , operation_(op)
              , value_(value)
              , mat_row_len_(rows)
              , mat_col_len_(cols)
            {
            }

            explicit mat_node(std::size_t matrix_id, ct::util::Operation op,
                mat_node const& other)
              : name_(matrix_id)
              , operation_(op)
              , copy_id_(other.name_)
              , mat_row_len_(other.mat_row_len_)
              , mat_col_len_(other.mat_col_len_)
            {
            }

            void pup(PUP::er& p)
            {
                p | name_;
                p | operation_;
                p | unary_expr_;
                p | binary_expr_;
                p | copy_id_;
                p | value_;
                p | mat_row_len_;
                p | mat_col_len_;
                p | left_;
                p | right_;
            }
        };

    }    // namespace mat_impl

}    // namespace ct

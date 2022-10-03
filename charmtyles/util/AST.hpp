#pragma once

#include <cstdint>
#include <iostream>
#include <vector>

#include "charm++.h"

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
            sub = 11
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
            }
        }

    }    // namespace util
    namespace vec_impl {
        struct vec_node
        {
            std::size_t name_ = -1;
            ct::util::Operation operation_;
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

            void pup(PUP::er& p)
            {
                p | name_;
                p | operation_;
                p | copy_id_;
                p | value_;
                p | vec_len_;
                p | left_;
                p | right_;
            }
        };

    }    // namespace vec_impl
}    // namespace ct
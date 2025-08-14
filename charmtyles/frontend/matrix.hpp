#pragma once

#include <charmtyles/backend/charmtyles_base.hpp>

#include <charmtyles/util/AST.hpp>
#include <charmtyles/util/singleton.hpp>
#include <charmtyles/util/sizes.hpp>

namespace ct {
    namespace unary_impl {
        template <typename Operand>
        class unary_expression;
    }
    namespace binary_impl {
        template <typename LeftOperand, typename RightOperand>
        class binary_expression;
    }    // namespace binary_impl
    namespace mat_impl {

        CT_GENERATE_SINGLETON(std::size_t, mat_shape_id);
        std::size_t get_mat_shape_id()
        {
            std::size_t& id = CT_ACCESS_SINGLETON(mat_shape_id);
            std::size_t curr_id = id++;

            return curr_id;
        }

        struct mat_shape_t
        {
            std::size_t shape_id;
            std::size_t matrix_id;
            std::size_t num_chares_x;
            std::size_t num_chares_y;
            CProxy_matrix_impl proxy;

            mat_shape_t() = default;

            explicit mat_shape_t(std::size_t matrix_id_,
                std::size_t num_chares_x_, std::size_t num_chares_y_,
                CProxy_matrix_impl proxy_)
              : shape_id(get_mat_shape_id())
              , matrix_id(matrix_id_)
              , num_chares_x(num_chares_x_)
              , num_chares_y(num_chares_y_)
              , proxy(proxy_)
            {
            }

            void pup(PUP::er& p)
            {
                p | shape_id;
                p | matrix_id;
                p | num_chares_x;
                p | num_chares_y;
                p | proxy;
            }
        };
        CT_GENERATE_SINGLETON(std::vector<mat_shape_t>, mat_shape_info);

        class mat_instr_queue_t
        {
        public:
            using ast_t = std::vector<mat_node>;
            using instr_t = std::vector<ast_t>;

            mat_instr_queue_t() = default;

            void insert(mat_node const& node, std::size_t shape_id)
            {
                shape_matrix_queue_[shape_id].emplace_back(ast_t{node});
            }

            void insert(ast_t const& ast, std::size_t shape_id)
            {
                shape_matrix_queue_[shape_id].emplace_back(ast);
            }

            void resize(std::size_t new_shape_id)
            {
                shape_matrix_queue_.resize(new_shape_id + 1);
                sdag_index_.resize(new_shape_id + 1);
            }

            std::size_t dispatch_size() const
            {
                std::size_t dispatch_count = 0;
                for (std::size_t i = 0; i != shape_matrix_queue_.size(); ++i)
                {
                    // Dispatch all non-empty vectors!
                    if (shape_matrix_queue_[i].size() != 0)
                    {
                        ++dispatch_count;
                    }
                }

                return dispatch_count;
            }

            void print_instructions() const
            {
                ckout << "Printing Instructions:" << endl;

                for (std::size_t i = 0; i != shape_matrix_queue_.size(); ++i)
                {
                    ckout << "Instructions for Shape ID: " << i << endl;

                    for (std::size_t num_instr = 0;
                        num_instr != shape_matrix_queue_[i].size(); ++num_instr)
                    {
                        ckout << "Instruction " << num_instr << ": ";
                        ct::util::parse_ast(
                            shape_matrix_queue_[i][num_instr], 0);
                        ckout << endl;
                    }
                }
            }

            void dispatch(ck::future<bool> is_done, CProxy_set_future proxy)
            {
                bool is_dispatched = false;
                for (std::size_t i = 0; i != shape_matrix_queue_.size(); ++i)
                {
                    // Dispatch all non-empty vectors!
                    if (shape_matrix_queue_[i].size() != 0)
                    {
                        is_dispatched = true;

                        std::size_t& sdag_index = sdag_index_[i];

                        CProxy_matrix_impl dispatch_proxy =
                            CT_ACCESS_SINGLETON(mat_shape_info)[i].proxy;

                        dispatch_proxy.compute(
                            sdag_index, shape_matrix_queue_[i], proxy);

                        ++sdag_index;
                        shape_matrix_queue_[i].clear();
                    }
                }

                if (!is_dispatched)
                    is_done.set(true);
            }

            void dispatch(std::size_t shape_id)
            {
                // Send instruction for execution
                if (shape_matrix_queue_[shape_id].size() != 0)
                {
                    std::size_t& sdag_index = sdag_index_[shape_id];

                    CProxy_matrix_impl dispatch_proxy =
                        CT_ACCESS_SINGLETON(mat_shape_info)[shape_id].proxy;

                    dispatch_proxy.compute(
                        sdag_index, shape_matrix_queue_[shape_id]);

                    ++sdag_index;
                    shape_matrix_queue_[shape_id].clear();
                }
            }

            void sync(std::size_t shape_id, CProxy_set_future proxy)
            {
                dispatch(shape_id);

                // Ensure synchronous threads are also saved
                std::size_t& sdag_index = sdag_index_[shape_id];
                CProxy_matrix_impl dispatch_proxy =
                    CT_ACCESS_SINGLETON(mat_shape_info)[shape_id].proxy;

                dispatch_proxy.synchronize(sdag_index, proxy);
                ++sdag_index;
            }

            std::size_t& sdag_idx(std::size_t shape_id)
            {
                return sdag_index_[shape_id];
            }

            const std::size_t& sdag_idx(std::size_t shape_id) const
            {
                return sdag_index_[shape_id];
            }

        private:
            std::vector<instr_t> shape_matrix_queue_;
            std::vector<std::size_t> sdag_index_;
        };
        CT_GENERATE_SINGLETON(mat_instr_queue_t, mat_instr_queue);

        ct::mat_impl::mat_shape_t get_mat_shape(
            std::size_t rows, std::size_t cols)
        {
            std::vector<ct::mat_impl::mat_shape_t>& shape_info =
                CT_ACCESS_SINGLETON(ct::mat_impl::mat_shape_info);

            std::size_t row_block_len =
                CT_ACCESS_SINGLETON(ct::util::matrix_block_rows);
            std::size_t col_block_len =
                CT_ACCESS_SINGLETON(ct::util::matrix_block_cols);

            std::size_t num_chares_x = cols / col_block_len;
            if (cols % col_block_len)
                ++num_chares_x;

            std::size_t num_chares_y = rows / row_block_len;
            if (rows % row_block_len)
                ++num_chares_y;

            auto it = std::find_if(shape_info.begin(), shape_info.end(),
                [num_chares_x, num_chares_y](
                    ct::mat_impl::mat_shape_t const& idx) {
                    return (idx.num_chares_x == num_chares_x &&
                        idx.num_chares_y == num_chares_y);
                });

            // No shape exists currently for given rows, cols
            if (it == shape_info.end())
            {
                // Create a new proxy for this shape and assign it to shape_info
                CProxy_matrix_impl proxy =
                    CProxy_matrix_impl::ckNew(num_chares_y, num_chares_x,
                        CT_ACCESS_SINGLETON(ct::util::matrix_block_rows),
                        CT_ACCESS_SINGLETON(ct::util::matrix_block_cols),
                        num_chares_x, num_chares_y);
                shape_info.emplace_back(ct::mat_impl::mat_shape_t{
                    0, num_chares_x, num_chares_y, proxy});

                ct::mat_impl::mat_shape_t matrix_shape = shape_info.back();

                ct::mat_impl::mat_shape_t& last = shape_info.back();
                last.matrix_id += 1;

                mat_instr_queue_t& queue = CT_ACCESS_SINGLETON(mat_instr_queue);
                queue.resize(matrix_shape.shape_id);

                return matrix_shape;
            }

            ct::mat_impl::mat_shape_t matrix_shape = *it;
            it->matrix_id += 1;

            return matrix_shape;
        }

        template <typename LHS, typename RHS>
        class mat_expression
        {
        public:
            explicit mat_expression(LHS const& lhs_, RHS const& rhs_,
                std::size_t rows_, std::size_t cols_, ct::util::Operation op_)
              : lhs(lhs_)
              , rhs(rhs_)
              , row_len(rows_)
              , col_len(cols_)
              , op(op_)
            {
            }

            std::vector<ct::mat_impl::mat_node> operator()() const
            {
                std::vector<ct::mat_impl::mat_node> left = lhs();
                std::vector<ct::mat_impl::mat_node> right = rhs();

                ct::mat_impl::mat_node node{op, row_len, col_len};

                node.left_ = 1;
                node.right_ = left.size() + 1;

                std::vector<ct::mat_impl::mat_node> ast;
                ast.reserve(left.size() + right.size() + 1);

                ast.emplace_back(node);
                std::copy(left.begin(), left.end(), std::back_inserter(ast));
                std::copy(right.begin(), right.end(), std::back_inserter(ast));

                // Update left and right neighbors
                for (int i = 1; i != left.size(); ++i)
                {
                    if (ast[i].left_ != static_cast<std::size_t>(-1))
                    {
                        ast[i].left_ += 1;
                    }

                    if (ast[i].right_ != static_cast<std::size_t>(-1))
                    {
                        ast[i].right_ += 1;
                    }
                }

                for (int i = 1 + left.size(); i != ast.size(); ++i)
                {
                    if (ast[i].left_ != static_cast<std::size_t>(-1))
                    {
                        ast[i].left_ += 1 + left.size();
                    }

                    if (ast[i].right_ != static_cast<std::size_t>(-1))
                    {
                        ast[i].right_ += 1 + left.size();
                    }
                }

                return ast;
            }

            std::size_t rows() const
            {
                return row_len;
            }

            std::size_t cols() const
            {
                return col_len;
            }

        private:
            LHS const& lhs;
            RHS const& rhs;
            std::size_t row_len;
            std::size_t col_len;
            ct::util::Operation op;
        };

    }    // namespace mat_impl

    namespace mat_mul_impl {
        class mat_mul_expr;
    }

    class matrix
    {
        template <typename LHS, typename RHS>
        friend class mat_impl::mat_expression;
        template <typename Operand>
        friend class unary_impl::unary_expression;
        template <typename LeftOperand, typename RightOperand>
        friend class binary_impl::binary_expression;

    public:
        matrix() = default;

        explicit matrix(std::size_t rows, std::size_t cols)
          : row_size_(rows)
          , col_size_(cols)
          , matrix_shape_(ct::mat_impl::get_mat_shape(rows, cols))
          , node_(matrix_shape_.matrix_id, ct::util::Operation::init_random,
                row_size_, col_size_)
        {
            ct::mat_impl::mat_instr_queue_t& queue =
                CT_ACCESS_SINGLETON(ct::mat_impl::mat_instr_queue);

            queue.insert(node_, matrix_shape_.shape_id);
        }

        explicit matrix(std::size_t rows, std::size_t cols, double value)
          : row_size_(rows)
          , col_size_(cols)
          , matrix_shape_(ct::mat_impl::get_mat_shape(rows, cols))
          , node_(matrix_shape_.matrix_id, ct::util::Operation::init_value,
                value, rows, cols)
        {
            ct::mat_impl::mat_instr_queue_t& queue =
                CT_ACCESS_SINGLETON(ct::mat_impl::mat_instr_queue);

            queue.insert(node_, matrix_shape_.shape_id);
        }

        explicit matrix(std::size_t rows, std::size_t cols,
            std::shared_ptr<ct::generator> gen_ptr)
          : row_size_(rows)
          , col_size_(cols)
          , matrix_shape_(ct::mat_impl::get_mat_shape(rows, cols))
          , node_(
                matrix_shape_.matrix_id, ct::util::Operation::noop, rows, cols)
        {
            ct::mat_impl::mat_instr_queue_t& queue =
                CT_ACCESS_SINGLETON(ct::mat_impl::mat_instr_queue);

            queue.dispatch(matrix_shape_.shape_id);

            std::size_t& sdag_idx = queue.sdag_idx(matrix_shape_.shape_id);

            matrix_shape_.proxy.generator_init(sdag_idx,
                matrix_shape_.matrix_id, row_size_, col_size_, gen_ptr);

            ++sdag_idx;
        }

        matrix(matrix const& other)
          : row_size_(other.row_size_)
          , col_size_(other.col_size_)
          , matrix_shape_(ct::mat_impl::get_mat_shape(row_size_, col_size_))
          , node_(
                matrix_shape_.matrix_id, ct::util::Operation::copy, other.node_)
        {
            ct::mat_impl::mat_instr_queue_t& queue =
                CT_ACCESS_SINGLETON(ct::mat_impl::mat_instr_queue);

            queue.insert(node_, matrix_shape_.shape_id);
        }

        matrix(
            matrix const& other, std::shared_ptr<unary_operator> unary_operator)
          : row_size_(other.row_size_)
          , col_size_(other.col_size_)
          , matrix_shape_(ct::mat_impl::get_mat_shape(row_size_, col_size_))
          , node_(other.matrix_shape_.matrix_id,
                ct::util::Operation::unary_expr, unary_operator, row_size_,
                col_size_)
        {
            ct::mat_impl::mat_instr_queue_t& queue =
                CT_ACCESS_SINGLETON(ct::mat_impl::mat_instr_queue);

            queue.insert(node_, matrix_shape_.shape_id);
        }

        matrix& operator=(matrix const& other)
        {
            node_.operation_ = ct::util::Operation::copy;
            node_.copy_id_ = other.node_.name_;

            ct::mat_impl::mat_instr_queue_t& queue =
                CT_ACCESS_SINGLETON(ct::mat_impl::mat_instr_queue);

            queue.insert(node_, matrix_shape_.shape_id);

            return *this;
        }

        matrix& operator+=(matrix const& other)
        {
            node_.operation_ = ct::util::Operation::inplace_add;
            node_.copy_id_ = other.node_.name_;

            ct::mat_impl::mat_instr_queue_t& queue =
                CT_ACCESS_SINGLETON(ct::mat_impl::mat_instr_queue);

            queue.insert(node_, matrix_shape_.shape_id);

            return *this;
        }

        matrix& operator-=(matrix const& other)
        {
            node_.operation_ = ct::util::Operation::inplace_sub;
            node_.copy_id_ = other.node_.name_;

            ct::mat_impl::mat_instr_queue_t& queue =
                CT_ACCESS_SINGLETON(ct::mat_impl::mat_instr_queue);

            queue.insert(node_, matrix_shape_.shape_id);

            return *this;
        }

        matrix& operator/=(matrix const& other)
        {
            node_.operation_ = ct::util::Operation::inplace_divide;
            node_.copy_id_ = other.node_.name_;

            ct::mat_impl::mat_instr_queue_t& queue =
                CT_ACCESS_SINGLETON(ct::mat_impl::mat_instr_queue);

            queue.insert(node_, matrix_shape_.shape_id);

            return *this;
        }

        // TODO: Figure out why this is necessary!
        matrix(matrix&& other)
          : row_size_(other.row_size_)
          , col_size_(other.col_size_)
          , matrix_shape_(other.matrix_shape_)
          , node_(other.node_)
        {
            // ckout << "Move constructor called!" << endl;
        }

        template <typename LHS, typename RHS>
        matrix(ct::mat_impl::mat_expression<LHS, RHS> const& e)
        {
            std::vector<ct::mat_impl::mat_node> instr = e();
            ct::mat_impl::mat_node& root = instr.front();
            row_size_ = root.mat_row_len_;
            col_size_ = root.mat_col_len_;

            matrix_shape_ = ct::mat_impl::get_mat_shape(row_size_, col_size_);

            root.name_ = matrix_shape_.matrix_id;
            node_ = ct::mat_impl::mat_node{root};

            ct::mat_impl::mat_instr_queue_t& queue =
                CT_ACCESS_SINGLETON(ct::mat_impl::mat_instr_queue);

            queue.insert(instr, matrix_shape_.shape_id);
        }

        matrix(ct::mat_mul_impl::mat_mul_expr const& expr);
        matrix& operator=(ct::mat_mul_impl::mat_mul_expr const& expr);

        template <typename Operand>
        matrix(ct::unary_impl::unary_expression<Operand> const& e);

        template <typename Operand>
        matrix& operator=(ct::unary_impl::unary_expression<Operand> const& e);

        template <typename LeftOperand, typename RightOperand>
        matrix(ct::binary_impl::binary_expression<LeftOperand, RightOperand> const& e);

        template <typename LeftOperand, typename RightOperand>
        matrix& operator=(ct::binary_impl::binary_expression<LeftOperand, RightOperand> const& e);

        template <typename LHS, typename RHS>
        matrix& operator=(ct::mat_impl::mat_expression<LHS, RHS> const& e)
        {
            std::vector<ct::mat_impl::mat_node> instr = e();
            ct::mat_impl::mat_node& root = instr.front();

            root.name_ = matrix_shape_.matrix_id;

            ct::mat_impl::mat_instr_queue_t& queue =
                CT_ACCESS_SINGLETON(ct::mat_impl::mat_instr_queue);

            queue.insert(instr, matrix_shape_.shape_id);

            return *this;
        }

        template <typename LHS, typename RHS>
        matrix& operator+=(ct::mat_impl::mat_expression<LHS, RHS> const& e)
        {
            std::vector<ct::mat_impl::mat_node> instr = e();
            ct::mat_impl::mat_node& root = instr.front();
            
            root.name_ = matrix_shape_.matrix_id;

            // make new dummy root and emplace it at the front
            ct::mat_impl::mat_node new_root{ct::util::Operation::inplace_add,
                row_size_, col_size_};
            new_root.left_ = -1;
            new_root.right_ = 1;
            new_root.mat_row_len_ = row_size_;
            new_root.mat_col_len_ = col_size_;
            new_root.name_ = matrix_shape_.matrix_id;
            instr.insert(instr.begin(), new_root);

            //make all the left and right if not -1 +=1 foe the other nodes
            for (std::size_t i = 1; i < instr.size(); ++i)
            {
                if (instr[i].left_ != static_cast<std::size_t>(-1))
                {
                    instr[i].left_ += 1;
                }
                if (instr[i].right_ != static_cast<std::size_t>(-1))
                {   
                    instr[i].right_ += 1;
                }
            }
            
            ct::mat_impl::mat_instr_queue_t& queue =
            CT_ACCESS_SINGLETON(ct::mat_impl::mat_instr_queue);
            queue.print_instructions();
            
            queue.insert(instr, matrix_shape_.shape_id);
            queue.print_instructions();

            return *this;
        }

        template <typename LHS, typename RHS>
        matrix& operator-=(ct::mat_impl::mat_expression<LHS, RHS> const& e)
        {
            std::vector<ct::mat_impl::mat_node> instr = e();
            ct::mat_impl::mat_node& root = instr.front();
            
            root.name_ = matrix_shape_.matrix_id;

            // make new dummy root and emplace it at the front
            ct::mat_impl::mat_node new_root{ct::util::Operation::inplace_sub,
                row_size_, col_size_};
            new_root.left_ = -1;
            new_root.right_ = 1;
            new_root.mat_row_len_ = row_size_;
            new_root.mat_col_len_ = col_size_;
            new_root.name_ = matrix_shape_.matrix_id;
            instr.insert(instr.begin(), new_root);

            //make all the left and right if not -1 +=1 foe the other nodes
            for (std::size_t i = 1; i < instr.size(); ++i)
            {
                if (instr[i].left_ != static_cast<std::size_t>(-1))
                {
                    instr[i].left_ += 1;
                }
                if (instr[i].right_ != static_cast<std::size_t>(-1))
                {   
                    instr[i].right_ += 1;
                }
            }
            
            ct::mat_impl::mat_instr_queue_t& queue =
            CT_ACCESS_SINGLETON(ct::mat_impl::mat_instr_queue);
            queue.print_instructions();
            
            queue.insert(instr, matrix_shape_.shape_id);
            queue.print_instructions();

            return *this;
        }
        
        template <typename LHS, typename RHS>
        matrix& operator/=(ct::mat_impl::mat_expression<LHS, RHS> const& e)
        {
            std::vector<ct::mat_impl::mat_node> instr = e();
            ct::mat_impl::mat_node& root = instr.front();
            
            root.name_ = matrix_shape_.matrix_id;

            // make new dummy root and emplace it at the front
            ct::mat_impl::mat_node new_root{ct::util::Operation::inplace_divide,
                row_size_, col_size_};
            new_root.left_ = -1;
            new_root.right_ = 1;
            new_root.mat_row_len_ = row_size_;
            new_root.mat_col_len_ = col_size_;
            new_root.name_ = matrix_shape_.matrix_id;
            instr.insert(instr.begin(), new_root);

            //make all the left and right if not -1 +=1 foe the other nodes
            for (std::size_t i = 1; i < instr.size(); ++i)
            {
                if (instr[i].left_ != static_cast<std::size_t>(-1))
                {
                    instr[i].left_ += 1;
                }
                if (instr[i].right_ != static_cast<std::size_t>(-1))
                {   
                    instr[i].right_ += 1;
                }
            }
            
            ct::mat_impl::mat_instr_queue_t& queue =
            CT_ACCESS_SINGLETON(ct::mat_impl::mat_instr_queue);
            queue.print_instructions();
            
            queue.insert(instr, matrix_shape_.shape_id);
            queue.print_instructions();

            return *this;
        }

    public:
        const ct::mat_impl::mat_shape_t matrix_shape() const
        {
            return matrix_shape_;
        }

        ct::mat_impl::mat_shape_t matrix_shape()
        {
            return matrix_shape_;
        }

        std::size_t rows() const
        {
            return row_size_;
        }

        std::size_t cols() const
        {
            return col_size_;
        }

        void pup(PUP::er& p)
        {
            p | row_size_;
            p | col_size_;
            p | matrix_shape_;
            p | node_;
        }

        std::vector<std::vector<double>> get()
        {
            ct::mat_impl::mat_instr_queue_t& queue =
                CT_ACCESS_SINGLETON(ct::mat_impl::mat_instr_queue);
            queue.dispatch(matrix_shape().shape_id);
            ck::future<std::vector<std::vector<double>>> fval;
            CProxy_get_mat_future mat_proxy =
                CProxy_get_mat_future::ckNew(fval, row_size_, col_size_);
            std::size_t& sdag_idx = queue.sdag_idx(matrix_shape().shape_id);
            matrix_shape().proxy.get_value(
                sdag_idx, matrix_shape().matrix_id, mat_proxy);
            ++sdag_idx;
            return fval.get();
        }

    private:
        std::vector<ct::mat_impl::mat_node> operator()() const
        {
            ct::mat_impl::mat_node new_node{node_};
            new_node.operation_ = ct::util::Operation::noop;

            return std::vector<ct::mat_impl::mat_node>{new_node};
        }

        std::size_t row_size_;
        std::size_t col_size_;
        ct::mat_impl::mat_shape_t matrix_shape_;
        ct::mat_impl::mat_node node_;
    };

    namespace traits {
        template <typename T>
        struct is_mat_type_impl
        {
            constexpr static bool value = false;
        };

        template <>
        struct is_mat_type_impl<ct::matrix>
        {
            constexpr static bool value = true;
        };

    }    // namespace traits

}    // namespace ct

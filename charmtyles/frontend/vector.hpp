#pragma once

#include <charmtyles/backend/charmtyles_base.hpp>

#include <charmtyles/util/AST.hpp>
#include <charmtyles/util/singleton.hpp>
#include <charmtyles/util/sizes.hpp>

#include <vector>

namespace ct {
    namespace vec_impl {

        CT_GENERATE_SINGLETON(std::size_t, vec_shape_id);
        std::size_t get_vec_shape_id()
        {
            std::size_t& id = CT_ACCESS_SINGLETON(vec_shape_id);
            std::size_t curr_id = id++;

            return curr_id;
        }

        struct vec_shape_t
        {
            std::size_t shape_id;
            std::size_t vector_id;
            std::size_t num_chares;
            CProxy_vector_impl proxy;

            // Only called when initializing vector through an expression
            vec_shape_t() = default;

            explicit vec_shape_t(std::size_t vector_id_,
                std::size_t num_chares_, CProxy_vector_impl proxy_)
              : shape_id(get_vec_shape_id())
              , vector_id(vector_id_)
              , num_chares(num_chares_)
              , proxy(proxy_)
            {
            }

            void pup(PUP::er& p)
            {
                p | shape_id;
                p | vector_id;
                p | num_chares;
                p | proxy;
            }
        };
        CT_GENERATE_SINGLETON(std::vector<vec_shape_t>, vec_shape_info);

        class vec_instr_queue_t
        {
        public:
            using ast_t = std::vector<vec_node>;
            using instr_t = std::vector<ast_t>;

            vec_instr_queue_t() = default;

            void insert(vec_node const& node, std::size_t shape_id)
            {
                shape_vector_queue_[shape_id].emplace_back(ast_t{node});
            }

            void insert(ast_t const& ast, std::size_t shape_id)
            {
                shape_vector_queue_[shape_id].emplace_back(ast);
            }

            void resize(std::size_t new_shape_id)
            {
                shape_vector_queue_.resize(new_shape_id + 1);
                sdag_index_.resize(new_shape_id + 1);
            }

            std::size_t dispatch_size() const
            {
                std::size_t dispatch_count = 0;
                for (std::size_t i = 0; i != shape_vector_queue_.size(); ++i)
                {
                    // Dispatch all non-empty vectors!
                    if (shape_vector_queue_[i].size() != 0)
                    {
                        ++dispatch_count;
                    }
                }

                return dispatch_count;
            }

            void print_instructions() const
            {
                ckout << "Printing Instructions:" << endl;

                for (std::size_t i = 0; i != shape_vector_queue_.size(); ++i)
                {
                    ckout << "Instructions for Shape ID: " << i << endl;

                    for (std::size_t num_instr = 0;
                         num_instr != shape_vector_queue_[i].size();
                         ++num_instr)
                    {
                        ckout << "Instruction " << num_instr << ": ";
                        ct::util::parse_ast(
                            shape_vector_queue_[i][num_instr], 0);
                        ckout << endl;
                    }
                }
            }

            void dispatch(ck::future<bool> is_done, CProxy_set_future proxy)
            {
                bool is_dispatched = false;
                for (std::size_t i = 0; i != shape_vector_queue_.size(); ++i)
                {
                    // Dispatch all non-empty vectors!
                    if (shape_vector_queue_[i].size() != 0)
                    {
                        is_dispatched = true;

                        std::size_t& sdag_index = sdag_index_[i];

                        CProxy_vector_impl dispatch_proxy =
                            CT_ACCESS_SINGLETON(vec_shape_info)[i].proxy;

                        dispatch_proxy.compute(
                            sdag_index, shape_vector_queue_[i], proxy);

                        ++sdag_index;
                        shape_vector_queue_[i].clear();
                    }
                }

                if (!is_dispatched)
                    is_done.set(true);
            }

            void dispatch(std::size_t shape_id)
            {
                // Send instructions for execution
                if (shape_vector_queue_[shape_id].size() != 0)
                {
                    std::size_t& sdag_index = sdag_index_[shape_id];

                    CProxy_vector_impl dispatch_proxy =
                        CT_ACCESS_SINGLETON(vec_shape_info)[shape_id].proxy;

                    dispatch_proxy.compute(
                        sdag_index, shape_vector_queue_[shape_id]);

                    ++sdag_index;
                    shape_vector_queue_[shape_id].clear();
                }
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
            // Shape -> Instructions -> AST (per instruction)
            std::vector<instr_t> shape_vector_queue_;
            std::vector<std::size_t> sdag_index_;
        };
        CT_GENERATE_SINGLETON(vec_instr_queue_t, vec_instr_queue);

        ct::vec_impl::vec_shape_t get_vector_shape(std::size_t size)
        {
            std::vector<ct::vec_impl::vec_shape_t>& shape_info =
                CT_ACCESS_SINGLETON(ct::vec_impl::vec_shape_info);

            std::size_t block_len =
                CT_ACCESS_SINGLETON(ct::util::array_block_len);

            std::size_t num_chares = size / block_len;

            if (size % block_len)
                ++num_chares;

            auto it = std::find_if(shape_info.begin(), shape_info.end(),
                [num_chares](ct::vec_impl::vec_shape_t const& idx) {
                    return (idx.num_chares == num_chares);
                });

            // No shape exists currently for given size
            if (it == shape_info.end())
            {
                // Create a new proxy for this shape and assign it to shape_info
                CProxy_vector_impl proxy = CProxy_vector_impl::ckNew(num_chares,
                    CT_ACCESS_SINGLETON(ct::util::array_block_len), num_chares);
                shape_info.emplace_back(
                    ct::vec_impl::vec_shape_t{0, num_chares, proxy});
                ct::vec_impl::vec_shape_t vector_shape = shape_info.back();

                ct::vec_impl::vec_shape_t& last = shape_info.back();
                last.vector_id += 1;

                vec_instr_queue_t& queue = CT_ACCESS_SINGLETON(vec_instr_queue);
                queue.resize(vector_shape.shape_id);

                return vector_shape;
            }

            ct::vec_impl::vec_shape_t vector_shape = *it;
            it->vector_id += 1;

            return vector_shape;
        }

        template <typename LHS, typename RHS>
        class vec_expression
        {
        public:
            explicit vec_expression(LHS const& lhs_, RHS const& rhs_,
                std::size_t vec_len_, ct::util::Operation op_)
              : lhs(lhs_)
              , rhs(rhs_)
              , vec_len(vec_len_)
              , op(op_)
            {
            }

            std::vector<ct::vec_impl::vec_node> operator()() const
            {
                std::vector<ct::vec_impl::vec_node> left = lhs();
                std::vector<ct::vec_impl::vec_node> right = rhs();

                ct::vec_impl::vec_node node{op, vec_len};

                node.left_ = 1;
                node.right_ = left.size() + 1;

                std::vector<ct::vec_impl::vec_node> ast;
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

            std::size_t size() const
            {
                return vec_len;
            }

        private:
            LHS const& lhs;
            RHS const& rhs;
            std::size_t vec_len;
            ct::util::Operation op;
        };

    }    // namespace vec_impl

    namespace dot_impl {
        class dot_expression;
    }

    namespace blas_impl {
        class vec_axpy_expr;
    }

    class vector
    {
        template <typename LHS, typename RHS>
        friend class vec_impl::vec_expression;

        friend class dot_impl::dot_expression;

    public:
        vector() = default;

        explicit vector(std::size_t size)
          : size_(size)
          , vector_shape_(ct::vec_impl::get_vector_shape(size))
          , node_(
                vector_shape_.vector_id, ct::util::Operation::init_random, size)
        {
            ct::vec_impl::vec_instr_queue_t& queue =
                CT_ACCESS_SINGLETON(ct::vec_impl::vec_instr_queue);

            queue.insert(node_, vector_shape_.shape_id);
        }

        explicit vector(std::size_t size, double value_)
          : size_(size)
          , vector_shape_(ct::vec_impl::get_vector_shape(size))
          , node_(vector_shape_.vector_id, ct::util::Operation::init_value,
                value_, size)
        {
            ct::vec_impl::vec_instr_queue_t& queue =
                CT_ACCESS_SINGLETON(ct::vec_impl::vec_instr_queue);

            queue.insert(node_, vector_shape_.shape_id);
        }

        vector(vector const& other)
          : size_(other.size_)
          , vector_shape_(ct::vec_impl::get_vector_shape(size_))
          , node_(
                vector_shape_.vector_id, ct::util::Operation::copy, other.node_)
        {
            ct::vec_impl::vec_instr_queue_t& queue =
                CT_ACCESS_SINGLETON(ct::vec_impl::vec_instr_queue);

            queue.insert(node_, vector_shape_.shape_id);
        }

        vector& operator=(vector const& other)
        {
            node_.operation_ = ct::util::Operation::copy;
            node_.copy_id_ = other.node_.name_;

            ct::vec_impl::vec_instr_queue_t& queue =
                CT_ACCESS_SINGLETON(ct::vec_impl::vec_instr_queue);

            queue.insert(node_, vector_shape_.shape_id);

            return *this;
        }

        // TODO: Figure out why this is necessary!
        vector(vector&& other)
        {
            ckout << "Move constructor called!" << endl;
        }

        template <typename LHS, typename RHS>
        vector(ct::vec_impl::vec_expression<LHS, RHS> const& e)
        {
            std::vector<ct::vec_impl::vec_node> instr = e();
            ct::vec_impl::vec_node& root = instr.front();
            size_ = root.vec_len_;

            vector_shape_ = ct::vec_impl::get_vector_shape(size_);

            root.name_ = vector_shape_.vector_id;
            node_ = ct::vec_impl::vec_node{root};

            ct::vec_impl::vec_instr_queue_t& queue =
                CT_ACCESS_SINGLETON(ct::vec_impl::vec_instr_queue);

            queue.insert(instr, vector_shape_.shape_id);
        }

        template <typename LHS, typename RHS>
        vector& operator=(ct::vec_impl::vec_expression<LHS, RHS> const& e)
        {
            std::vector<ct::vec_impl::vec_node> instr = e();
            ct::vec_impl::vec_node& root = instr.front();

            root.name_ = vector_shape_.vector_id;

            ct::vec_impl::vec_instr_queue_t& queue =
                CT_ACCESS_SINGLETON(ct::vec_impl::vec_instr_queue);

            queue.insert(instr, vector_shape_.shape_id);

            return *this;
        }

        vector(dot_impl::dot_expression const&);
        vector& operator=(dot_impl::dot_expression const&);

        vector(blas_impl::vec_axpy_expr const&);
        vector& operator=(blas_impl::vec_axpy_expr const&);

        // Helper functions
    public:
        const ct::vec_impl::vec_shape_t vector_shape() const
        {
            return vector_shape_;
        }

        ct::vec_impl::vec_shape_t vector_shape()
        {
            return vector_shape_;
        }

        std::size_t size() const
        {
            return size_;
        }

        void pup(PUP::er& p)
        {
            p | size_;
            p | vector_shape_;
            p | node_;
        }

    private:
        std::vector<ct::vec_impl::vec_node> operator()() const
        {
            ct::vec_impl::vec_node new_node{node_};
            new_node.operation_ = ct::util::Operation::noop;

            return std::vector<ct::vec_impl::vec_node>{new_node};
        }

        std::size_t size_;
        ct::vec_impl::vec_shape_t vector_shape_;
        ct::vec_impl::vec_node node_;
    };

    namespace traits {
        template <typename T>
        struct is_vec_type_impl
        {
            constexpr static bool value = false;
        };

        template <>
        struct is_vec_type_impl<ct::vector>
        {
            constexpr static bool value = true;
        };

    }    // namespace traits

}    // namespace ct

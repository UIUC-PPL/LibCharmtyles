#pragma once

#include <charmtyles/frontend/scalar.hpp>
#include <charmtyles/frontend/vector.hpp>

#include <stdexcept>
#include <type_traits>
#include <concepts>

namespace ct {
    namespace traits {
        namespace internal {
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
    
            template <typename... Ts>
            struct is_vec_type_impl<ct::vec_impl::vec_expression<Ts...>>
            {
                constexpr static bool value = true;
            };
    
            template <typename... Ts>
            struct is_vec_type_impl<ct::vec_impl::ter_vec_expression<Ts...>>
            {
                constexpr static bool value = true;
            };
        }

        template<typename T>
        constexpr static bool is_vec_type_v = internal::is_vec_type_impl<T>::value;

        template<typename LHS, typename RHS>
        constexpr static bool is_bin_vec_type_v = is_vec_type_v<LHS> && is_vec_type_v<RHS>;

        template<typename LHS, typename RHS, typename THS>
        constexpr static bool is_ter_vec_type_v = is_bin_vec_type_v<LHS, RHS> && is_vec_type_v<THS>;

        namespace internal {
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
    
            template <typename... Ts>
            struct is_mat_type_impl<ct::mat_impl::mat_expression<Ts...>>
            {
                constexpr static bool value = true;
            };
    
            template <typename... Ts>
            struct is_mat_type_impl<ct::mat_impl::ter_mat_expression<Ts...>>
            {
                constexpr static bool value = true;
            };
        }

        template<typename T>
        constexpr static bool is_mat_type_v = internal::is_mat_type_impl<T>::value;

        template<typename LHS, typename RHS>
        constexpr static bool is_bin_mat_type_v = is_mat_type_v<LHS> && is_mat_type_v<RHS>;

        template<typename LHS, typename RHS, typename THS>
        constexpr static bool is_ter_mat_type_v = is_bin_mat_type_v<LHS, RHS> && is_mat_type_v<THS>;

        template<typename T>
        concept is_tensor_type = is_vec_type_v<std::decay_t<T>> || is_mat_type_v<std::decay_t<T>>;
    }

    template <typename LHS, typename RHS>
    auto inline operator_impl(LHS const& lhs, RHS const& rhs, ct::util::Operation op)
    {
        using LHS_T = std::decay_t<LHS>;
        using RHS_T = std::decay_t<RHS>;
        
        if constexpr (ct::traits::is_bin_vec_type_v<LHS, RHS>)
        {
            return ct::vec_impl::vec_expression<LHS, RHS>{lhs, rhs, lhs.size(), op};
        }
        else if constexpr (ct::traits::is_bin_mat_type_v<LHS, RHS>)
        {
            return ct::mat_impl::mat_expression<LHS, RHS>{lhs, rhs, lhs.rows(), lhs.cols(), op};
        }
        else if constexpr (ct::traits::is_vec_type_v<LHS_T> || ct::traits::is_vec_type_v<RHS_T>)
        {
            if constexpr (std::is_arithmetic_v<LHS_T>)
            {
                return ct::vec_impl::vec_expression<RHS, RHS>{lhs, rhs, rhs.size(), op};
            }
            else if constexpr (std::is_same_v<LHS_T, ct::scalar>)
            {
                return ct::vec_impl::vec_expression<RHS, RHS>{
                    lhs.get(), rhs, rhs.size(), op};
            }
            else if constexpr (std::is_arithmetic_v<RHS_T>)
            {
                return ct::vec_impl::vec_expression<LHS, LHS>{lhs, rhs, lhs.size(), op};
            }
            else if constexpr (std::is_same_v<RHS_T,ct::scalar>)
            {
                return ct::vec_impl::vec_expression<LHS, LHS>{lhs, rhs.get(), lhs.size(), op};
            }
            else
            {
                CkAbort("Vectors to matrix broadcasting not yet supported");
            }
        }
        else if constexpr (ct::traits::is_mat_type_v<LHS_T> || ct::traits::is_mat_type_v<RHS_T>)
        {
            if constexpr (std::is_arithmetic_v<LHS_T>)
            {
                return ct::mat_impl::mat_expression<RHS, RHS>{lhs, rhs, rhs.rows(), rhs.cols(), op};
            }
            else if constexpr (std::is_same_v<LHS_T, ct::scalar>)
            {
                return ct::mat_impl::mat_expression<RHS, RHS>{lhs.get(), rhs, rhs.rows(), rhs.cols(), op};
            }
            else if constexpr (std::is_arithmetic_v<RHS_T>)
            {
                return ct::mat_impl::mat_expression<LHS, LHS>{lhs, rhs, lhs.rows(), lhs.cols(), op};
            }
            else if constexpr (std::is_same_v<RHS_T, ct::scalar>)
            {
                return ct::mat_impl::mat_expression<LHS, LHS>{lhs, rhs.get(), lhs.rows(), lhs.cols(), op};
            }
        }
    }

    template <typename LHS, typename RHS> 
    requires ct::traits::is_tensor_type<LHS> || ct::traits::is_tensor_type<RHS>
    auto operator+(LHS const& lhs, RHS const& rhs)
    {
        return operator_impl(lhs, rhs, ct::util::Operation::add);
    }

    template <typename LHS, typename RHS>
    requires ct::traits::is_tensor_type<LHS> || ct::traits::is_tensor_type<RHS>
    auto operator-(LHS const& lhs, RHS const& rhs)
    {
        return operator_impl(lhs, rhs, ct::util::Operation::sub);
    }

    template <typename LHS, typename RHS>
    requires ct::traits::is_tensor_type<LHS> || ct::traits::is_tensor_type<RHS>
    auto operator/(LHS const& lhs, RHS const& rhs)
    {
        return operator_impl(lhs, rhs, ct::util::Operation::divide);
    }

    template <typename LHS, typename RHS>
    requires ct::traits::is_tensor_type<LHS> || ct::traits::is_tensor_type<RHS>
    auto operator*(LHS const& lhs, RHS const& rhs)
    {
        return operator_impl(lhs, rhs, ct::util::Operation::multiply);
    }

    template <typename LHS, typename RHS>
    requires ct::traits::is_tensor_type<LHS> || ct::traits::is_tensor_type<RHS>
    auto operator>(LHS const& lhs, RHS const& rhs)
    {
        return operator_impl(lhs, rhs, ct::util::Operation::greater);
    }

    template <typename LHS, typename RHS>
    requires ct::traits::is_tensor_type<LHS> || ct::traits::is_tensor_type<RHS>
    auto operator<(LHS const& lhs, RHS const& rhs)
    {
        return operator_impl(lhs, rhs, ct::util::Operation::lesser);
    }

    template <typename LHS, typename RHS>
    requires ct::traits::is_tensor_type<LHS> || ct::traits::is_tensor_type<RHS>
    auto operator==(LHS const& lhs, RHS const& rhs)
    {
        return operator_impl(lhs, rhs, ct::util::Operation::eq);
    }

    template <typename LHS, typename RHS>
    requires ct::traits::is_tensor_type<LHS> || ct::traits::is_tensor_type<RHS>
    auto operator!=(LHS const& lhs, RHS const& rhs)
    {
        return operator_impl(lhs, rhs, ct::util::Operation::neq);
    }

    template <typename LHS, typename RHS>
    requires ct::traits::is_tensor_type<LHS> || ct::traits::is_tensor_type<RHS>
    auto operator>=(LHS const& lhs, RHS const& rhs)
    {
        return operator_impl(lhs, rhs, ct::util::Operation::geq);
    }

    template <typename LHS, typename RHS>
    requires ct::traits::is_tensor_type<LHS> || ct::traits::is_tensor_type<RHS>
    auto operator<=(LHS const& lhs, RHS const& rhs)
    {
        return operator_impl(lhs, rhs, ct::util::Operation::leq);
    }

    template <typename LHS, typename RHS>
    requires ct::traits::is_tensor_type<LHS> || ct::traits::is_tensor_type<RHS>
    auto operator&&(LHS const& lhs, RHS const& rhs)
    {
        return operator_impl(lhs, rhs, ct::util::Operation::logical_and);
    }

    template <typename LHS, typename RHS>
    requires ct::traits::is_tensor_type<LHS> || ct::traits::is_tensor_type<RHS>
    auto operator||(LHS const& lhs, RHS const& rhs)
    {
        return operator_impl(lhs, rhs, ct::util::Operation::logical_or);
    }

    template <typename LHS>
    requires ct::traits::is_tensor_type<LHS>
    auto operator!(LHS const& lhs)
    {
        return operator_impl(lhs, lhs, ct::util::Operation::logical_not);
    }

    inline ct::scalar dot(ct::vector const& lhs, ct::vector const& rhs)
    {
        std::size_t lhs_shape_id = lhs.vector_shape().shape_id;
        std::size_t rhs_shape_id = rhs.vector_shape().shape_id;
        CkAssert(lhs_shape_id == rhs_shape_id &&
            "Dot product across vectors belonging to different shapes is "
            "illegal.");

        // Dispatch previous instructions belonging to this shape
        ct::vec_impl::vec_instr_queue_t& queue =
            CT_ACCESS_SINGLETON(ct::vec_impl::vec_instr_queue);
        queue.dispatch(lhs_shape_id);

        ct::scalar result;

        std::size_t& scal_sdag_idx =
            CT_ACCESS_SINGLETON(ct::scal_impl::scalar_sdag_idx);
        std::size_t& vec_sdag_idx =
            CT_ACCESS_SINGLETON(ct::vec_impl::vec_instr_queue)
                .sdag_idx(lhs_shape_id);

        CProxy_vector_impl dispatch_proxy = lhs.vector_shape().proxy;
        dispatch_proxy.dot(vec_sdag_idx, lhs.vector_shape().vector_id,
            rhs.vector_shape().vector_id, scal_sdag_idx);
        scalar_impl_proxy.update_scalar(scal_sdag_idx, result.scalar_id());

        // Increment SDAG counters since operation has finished
        ++scal_sdag_idx;
        ++vec_sdag_idx;

        return result;
    }

    namespace dot_impl {

        class dot_expression
        {
            friend class ct::vector;

        public:
            dot_expression(ct::vector const& lhs_, ct::matrix const& rhs_,
                bool vec_mat = true)
              : lhs(lhs_)
              , rhs(rhs_)
            {
            }

            std::size_t cols() const
            {
                return rhs.cols();
            }

            std::size_t rows() const
            {
                return rhs.rows();
            }

        private:
            ct::vector const& lhs;
            ct::matrix const& rhs;
            bool is_vec_mat;
        };
    }    // namespace dot_impl

    inline vector::vector(dot_impl::dot_expression const& expr)
      : size_(expr.rows())
      , vector_shape_(ct::vec_impl::get_vector_shape(size_))
      , node_(vector_shape_.vector_id, ct::util::Operation::noop, size_)
    {
        ct::vec_impl::vec_shape_t const& lhs_shape = expr.lhs.vector_shape();
        ct::mat_impl::mat_shape_t const& rhs_shape = expr.rhs.matrix_shape();

        // Dispatch previous instructions belonging to this shape
        ct::vec_impl::vec_instr_queue_t& vec_queue =
            CT_ACCESS_SINGLETON(ct::vec_impl::vec_instr_queue);
        vec_queue.dispatch(lhs_shape.shape_id);

        ct::mat_impl::mat_instr_queue_t& mat_queue =
            CT_ACCESS_SINGLETON(ct::mat_impl::mat_instr_queue);
        mat_queue.dispatch(rhs_shape.shape_id);

        // Dispatch all vector in from the resultant vector's shape
        vec_queue.dispatch(vector_shape_.shape_id);

        std::size_t& lhs_sdag_idx = vec_queue.sdag_idx(lhs_shape.shape_id);
        std::size_t& rhs_sdag_idx = mat_queue.sdag_idx(rhs_shape.shape_id);
        std::size_t& result_sdag_idx =
            vec_queue.sdag_idx(vector_shape_.shape_id);

        CProxy_matrix_impl dispatch_proxy = rhs_shape.proxy;
        CProxy_vector_impl lhs_proxy = lhs_shape.proxy;

        lhs_proxy.send_to_matrix(
            lhs_sdag_idx, lhs_shape.vector_id, rhs_sdag_idx, dispatch_proxy);

        if (expr.is_vec_mat)
            dispatch_proxy.vec_mat_dot(rhs_sdag_idx, rhs_shape.matrix_id,
                result_sdag_idx, vector_shape_.proxy, vector_shape_.vector_id,
                size_);
        else
            dispatch_proxy.mat_vec_dot(rhs_sdag_idx, rhs_shape.matrix_id,
                result_sdag_idx, vector_shape_.proxy, vector_shape_.vector_id,
                size_);

        if (lhs_shape.shape_id == vector_shape_.shape_id)
            vector_shape_.proxy.update_index(
                result_sdag_idx + 1, vector_shape_.vector_id);
        else
            vector_shape_.proxy.update_index(
                result_sdag_idx, vector_shape_.vector_id);

        ++lhs_sdag_idx;
        ++rhs_sdag_idx;
        ++result_sdag_idx;
    }

    inline vector& vector::operator=(dot_impl::dot_expression const& expr)
    {
        ct::vec_impl::vec_shape_t const& lhs_shape = expr.lhs.vector_shape();
        ct::mat_impl::mat_shape_t const& rhs_shape = expr.rhs.matrix_shape();

        // Dispatch previous instructions belonging to this shape
        ct::vec_impl::vec_instr_queue_t& vec_queue =
            CT_ACCESS_SINGLETON(ct::vec_impl::vec_instr_queue);
        vec_queue.dispatch(lhs_shape.shape_id);

        ct::mat_impl::mat_instr_queue_t& mat_queue =
            CT_ACCESS_SINGLETON(ct::mat_impl::mat_instr_queue);
        mat_queue.dispatch(rhs_shape.shape_id);

        // Dispatch all vector in from the resultant vector's shape
        vec_queue.dispatch(vector_shape_.shape_id);

        std::size_t& lhs_sdag_idx = vec_queue.sdag_idx(lhs_shape.shape_id);
        std::size_t& rhs_sdag_idx = mat_queue.sdag_idx(rhs_shape.shape_id);
        std::size_t& result_sdag_idx =
            vec_queue.sdag_idx(vector_shape_.shape_id);

        CProxy_matrix_impl dispatch_proxy = rhs_shape.proxy;
        CProxy_vector_impl lhs_proxy = lhs_shape.proxy;

        lhs_proxy.send_to_matrix(
            lhs_sdag_idx, lhs_shape.vector_id, rhs_sdag_idx, dispatch_proxy);
        if (expr.is_vec_mat)
            dispatch_proxy.vec_mat_dot(rhs_sdag_idx, rhs_shape.matrix_id,
                result_sdag_idx, vector_shape_.proxy, vector_shape_.vector_id,
                size_);
        else
            dispatch_proxy.mat_vec_dot(rhs_sdag_idx, rhs_shape.matrix_id,
                result_sdag_idx, vector_shape_.proxy, vector_shape_.vector_id,
                size_);

        if (lhs_shape.shape_id == vector_shape_.shape_id)
            vector_shape_.proxy.update_index(
                result_sdag_idx + 1, vector_shape_.vector_id);
        else
            vector_shape_.proxy.update_index(
                result_sdag_idx, vector_shape_.vector_id);

        ++lhs_sdag_idx;
        ++rhs_sdag_idx;
        ++result_sdag_idx;

        return *this;
    }

    inline ct::dot_impl::dot_expression dot(
        ct::vector const& lhs, ct::matrix const& rhs)
    {
        std::size_t lhs_len = lhs.size();
        std::size_t rhs_cols = rhs.cols();
        CkAssert(lhs_len == rhs_cols && "Invalid dot product dimensions.");

        return ct::dot_impl::dot_expression{lhs, rhs};
    }

    inline ct::dot_impl::dot_expression dot(
        ct::matrix const& lhs, ct::vector const& rhs)
    {
        std::size_t lhs_rows = lhs.rows();
        std::size_t rhs_len = rhs.size();
        CkAssert(rhs_len == lhs_rows && "Invalid dot product dimensions.");

        return ct::dot_impl::dot_expression{rhs, lhs, false};
    }

    // Non-implemented dot product types
    inline void dot(ct::vector const& lhs, ct::matrix&& rhs)
    {
        CkAbort(
            "Dot Product with rvalue reference parameter is not supported.");
    }

    inline void dot(ct::vector&& lhs, ct::matrix&& rhs)
    {
        CkAbort(
            "Dot Product with rvalue reference parameter is not supported.");
    }

    inline void dot(ct::vector&& lhs, ct::matrix const& rhs)
    {
        CkAbort(
            "Dot Product with rvalue reference parameter is not supported.");
    }

    inline void dot(ct::matrix const& lhs, ct::vector&& rhs)
    {
        CkAbort(
            "Dot Product with rvalue reference parameter is not supported.");
    }

    inline void dot(ct::matrix&& lhs, ct::vector&& rhs)
    {
        CkAbort(
            "Dot Product with rvalue reference parameter is not supported.");
    }

    inline void dot(ct::matrix&& lhs, ct::vector const& rhs)
    {
        CkAbort(
            "Dot Product with rvalue reference parameter is not supported.");
    }
    
    namespace mat_mul_impl {
        class mat_mul_expr
        {
            friend class ct::matrix;

        public:
            mat_mul_expr(ct::matrix const& lhs_, ct::matrix const& rhs_)
              : lhs(lhs_)
              , rhs(rhs_)
            {
            }

            std::size_t rows() const
            {
                return lhs.rows();
            }

            std::size_t cols() const
            {
                return lhs.cols();
            }

        private:
            ct::matrix const& lhs;
            ct::matrix const& rhs;
        };
    }    // namespace mat_mul_impl

    inline matrix::matrix(ct::mat_mul_impl::mat_mul_expr const& expr)
      : row_size_(expr.rows())
      , col_size_(expr.cols())
      , matrix_shape_(ct::mat_impl::get_mat_shape(row_size_, col_size_))
      , node_(matrix_shape_.matrix_id, ct::util::Operation::init_value, 0,
            row_size_, col_size_)
    {
        ct::mat_impl::mat_instr_queue_t& mat_queue = CT_ACCESS_SINGLETON(ct::mat_impl::mat_instr_queue);
        mat_queue.insert(node_, matrix_shape_.shape_id);
        mat_queue.dispatch(matrix_shape_.shape_id);

        ct::mat_impl::mat_shape_t const& lhs_shape = expr.lhs.matrix_shape();
        ct::mat_impl::mat_shape_t const& rhs_shape = expr.rhs.matrix_shape();
        std::size_t& curr_sdag_idx = mat_queue.sdag_idx(matrix_shape_.shape_id);

        CProxy_matrix_impl dispatch_proxy = matrix_shape_.proxy;
        dispatch_proxy.mat_mat_mul(curr_sdag_idx, matrix_shape_.matrix_id,
            lhs_shape.matrix_id, rhs_shape.matrix_id);

        ++curr_sdag_idx;
    }

    inline matrix& matrix::operator=(ct::mat_mul_impl::mat_mul_expr const& expr)
    {
        CkAssert(expr.rows() == rows() && expr.cols() == cols() &&
            "Mismatched matrix dimensions");

        ct::mat_impl::mat_shape_t const& lhs_shape = expr.lhs.matrix_shape();
        ct::mat_impl::mat_shape_t const& rhs_shape = expr.rhs.matrix_shape();

        // Dispatch previous instructions belonging to this shape
        ct::mat_impl::mat_instr_queue_t& mat_queue =
            CT_ACCESS_SINGLETON(ct::mat_impl::mat_instr_queue);
        mat_queue.dispatch(matrix_shape_.shape_id);

        std::size_t& curr_sdag_idx = mat_queue.sdag_idx(matrix_shape_.shape_id);

        CProxy_matrix_impl dispatch_proxy = matrix_shape_.proxy;
        dispatch_proxy.mat_mat_mul(curr_sdag_idx, matrix_shape_.matrix_id,
            lhs_shape.matrix_id, rhs_shape.matrix_id);

        ++curr_sdag_idx;

        return *this;
    }

    inline ct::mat_mul_impl::mat_mul_expr matmul(matrix const& lhs, matrix const& rhs) {
        return mat_mul_impl::mat_mul_expr(lhs, rhs);
    }

    inline mat_mul_impl::mat_mul_expr matmul(matrix const& lhs, matrix&& rhs)
    {
        CkAbort(
            "Matrix Multiplication not implemented for complex operations.");
    }

    inline mat_mul_impl::mat_mul_expr matmul(matrix&& lhs, matrix const& rhs)
    {
        CkAbort(
            "Matrix Multiplication not implemented for complex operations.");
    }

    inline mat_mul_impl::mat_mul_expr matmul(matrix&& lhs, matrix&& rhs)
    {
        CkAbort(
            "Matrix Multiplication not implemented for complex operations.");
    }

    inline ct::scalar sum(ct::vector const& vec)
    {
        ct::vec_impl::vec_shape_t vec_info = vec.vector_shape();

        // Dispatch previous instructions belonging to this shape
        ct::vec_impl::vec_instr_queue_t& queue =
            CT_ACCESS_SINGLETON(ct::vec_impl::vec_instr_queue);
        queue.dispatch(vec_info.shape_id);

        ct::scalar result;

        std::size_t& scal_sdag_idx =
            CT_ACCESS_SINGLETON(ct::scal_impl::scalar_sdag_idx);
        std::size_t& vec_sdag_idx = queue.sdag_idx(vec_info.shape_id);

        CProxy_vector_impl dispatch_proxy = vec_info.proxy;
        dispatch_proxy.reduce_sum(
            vec_sdag_idx, vec_info.vector_id, scal_sdag_idx);
        scalar_impl_proxy.update_scalar(scal_sdag_idx, result.scalar_id());

        ++scal_sdag_idx;
        ++vec_sdag_idx;

        return result;
    }

    inline ct::scalar sum(ct::matrix const& mat)
    {
        ct::mat_impl::mat_shape_t mat_info = mat.matrix_shape();

        // Dispatch previous instructions belonging to this shape
        ct::mat_impl::mat_instr_queue_t& queue =
            CT_ACCESS_SINGLETON(ct::mat_impl::mat_instr_queue);
        queue.dispatch(mat_info.shape_id);

        ct::scalar result;

        std::size_t& scal_sdag_idx =
            CT_ACCESS_SINGLETON(ct::scal_impl::scalar_sdag_idx);
        std::size_t& mat_sdag_idx = queue.sdag_idx(mat_info.shape_id);

        CProxy_matrix_impl dispatch_proxy = mat_info.proxy;
        dispatch_proxy.reduce_sum(
            mat_sdag_idx, mat_info.matrix_id, scal_sdag_idx);
        scalar_impl_proxy.update_scalar(scal_sdag_idx, result.scalar_id());

        ++scal_sdag_idx;
        ++mat_sdag_idx;

        return result;
    }

    inline ct::scalar squared_norm(ct::vector const& vec)
    {
        ct::vec_impl::vec_shape_t vec_info = vec.vector_shape();

        // Dispatch previous instructions belonging to this shape
        ct::vec_impl::vec_instr_queue_t& queue =
            CT_ACCESS_SINGLETON(ct::vec_impl::vec_instr_queue);
        queue.dispatch(vec_info.shape_id);

        ct::scalar result;

        std::size_t& scal_sdag_idx =
            CT_ACCESS_SINGLETON(ct::scal_impl::scalar_sdag_idx);
        std::size_t& vec_sdag_idx = queue.sdag_idx(vec_info.shape_id);

        CProxy_vector_impl dispatch_proxy = vec_info.proxy;
        dispatch_proxy.norm_p(
            vec_sdag_idx, vec_info.vector_id, 2, scal_sdag_idx);
        scalar_impl_proxy.norm_update(scal_sdag_idx, result.scalar_id(), 2);

        ++scal_sdag_idx;
        ++vec_sdag_idx;

        return result;
    }

    inline ct::scalar norm_p(std::size_t p, ct::vector const& vec)
    {
        ct::vec_impl::vec_shape_t vec_info = vec.vector_shape();

        // Dispatch previous instructions belonging to this shape
        ct::vec_impl::vec_instr_queue_t& queue =
            CT_ACCESS_SINGLETON(ct::vec_impl::vec_instr_queue);
        queue.dispatch(vec_info.shape_id);

        ct::scalar result;

        std::size_t& scal_sdag_idx =
            CT_ACCESS_SINGLETON(ct::scal_impl::scalar_sdag_idx);
        std::size_t& vec_sdag_idx = queue.sdag_idx(vec_info.shape_id);

        CProxy_vector_impl dispatch_proxy = vec_info.proxy;
        dispatch_proxy.norm_p(
            vec_sdag_idx, vec_info.vector_id, p, scal_sdag_idx);
        scalar_impl_proxy.norm_update(scal_sdag_idx, result.scalar_id(), p);

        ++scal_sdag_idx;
        ++vec_sdag_idx;

        return result;
    }

    inline ct::scalar min(ct::vector const& vec)
    {
        ct::vec_impl::vec_shape_t vec_info = vec.vector_shape();

        // Dispatch previous instructions belonging to this shape
        ct::vec_impl::vec_instr_queue_t& queue =
            CT_ACCESS_SINGLETON(ct::vec_impl::vec_instr_queue);
        queue.dispatch(vec_info.shape_id);

        ct::scalar result;

        std::size_t& scal_sdag_idx =
            CT_ACCESS_SINGLETON(ct::scal_impl::scalar_sdag_idx);
        std::size_t& vec_sdag_idx = queue.sdag_idx(vec_info.shape_id);

        CProxy_vector_impl dispatch_proxy = vec_info.proxy;
        dispatch_proxy.min(vec_sdag_idx, vec_info.vector_id, scal_sdag_idx);
        scalar_impl_proxy.update_scalar(scal_sdag_idx, result.scalar_id());

        ++scal_sdag_idx;
        ++vec_sdag_idx;

        return result;
    }

    inline ct::scalar max(ct::vector const& vec)
    {
        ct::vec_impl::vec_shape_t vec_info = vec.vector_shape();

        // Dispatch previous instructions belonging to this shape
        ct::vec_impl::vec_instr_queue_t& queue =
            CT_ACCESS_SINGLETON(ct::vec_impl::vec_instr_queue);
        queue.dispatch(vec_info.shape_id);

        ct::scalar result;

        std::size_t& scal_sdag_idx =
            CT_ACCESS_SINGLETON(ct::scal_impl::scalar_sdag_idx);
        std::size_t& vec_sdag_idx = queue.sdag_idx(vec_info.shape_id);

        CProxy_vector_impl dispatch_proxy = vec_info.proxy;
        dispatch_proxy.max(vec_sdag_idx, vec_info.vector_id, scal_sdag_idx);
        scalar_impl_proxy.update_scalar(scal_sdag_idx, result.scalar_id());

        ++scal_sdag_idx;
        ++vec_sdag_idx;

        return result;
    }

    inline ct::vector get_avg(ct::vector const& vec, std::size_t k)
    {
        if (k == 0)
        {
            throw std::invalid_argument("k must be greater than 0");
        }
        if (k >= vec.size())
        {
            return vec;    // Return the original vector
        }

        ct::vec_impl::vec_shape_t vec_info = vec.vector_shape();

        ct::vec_impl::vec_instr_queue_t& queue =
            CT_ACCESS_SINGLETON(ct::vec_impl::vec_instr_queue);
        queue.dispatch(vec_info.shape_id);

        ck::future<std::vector<double>> fval;
        CProxy_get_partial_vec_future vec_proxy =
            CProxy_get_partial_vec_future::ckNew(fval, k);

        std::size_t& vec_sdag_idx = queue.sdag_idx(vec_info.shape_id);

        CProxy_vector_impl dispatch_proxy = vec_info.proxy;
        dispatch_proxy.get_avg_chunks(vec_sdag_idx, vec_info.vector_id,
            static_cast<int>(k), static_cast<int>(vec.size()), vec_proxy);

        ++vec_sdag_idx;

        std::vector<double> chunk_avgs = fval.get();

        std::shared_ptr<ct::from_vector_generator> gen =
            std::make_shared<ct::from_vector_generator>(chunk_avgs);
        ct::vector result{k, gen};

        return result;
    }

    inline ct::vector get_max(ct::vector const& vec, std::size_t k)
    {
        if (k == 0)
        {
            throw std::invalid_argument("k must be greater than 0");
        }
        if (k >= vec.size())
        {
            return vec;    // Return the original vector
        }

        ct::vec_impl::vec_shape_t vec_info = vec.vector_shape();

        ct::vec_impl::vec_instr_queue_t& queue =
            CT_ACCESS_SINGLETON(ct::vec_impl::vec_instr_queue);
        queue.dispatch(vec_info.shape_id);

        ck::future<std::vector<double>> fval;
        CProxy_get_partial_vec_future vec_proxy =
            CProxy_get_partial_vec_future::ckNew(fval, k);

        std::size_t& vec_sdag_idx = queue.sdag_idx(vec_info.shape_id);

        CProxy_vector_impl dispatch_proxy = vec_info.proxy;
        dispatch_proxy.get_max_chunks(vec_sdag_idx, vec_info.vector_id,
            static_cast<int>(k), static_cast<int>(vec.size()), vec_proxy);

        ++vec_sdag_idx;

        std::vector<double> chunk_maxs = fval.get();

        std::shared_ptr<ct::from_vector_generator> gen =
            std::make_shared<ct::from_vector_generator>(chunk_maxs);
        ct::vector result{k, gen};

        return result;
    }

    inline ct::vector get_min(ct::vector const& vec, std::size_t k)
    {
        if (k == 0)
        {
            throw std::invalid_argument("k must be greater than 0");
        }
        if (k >= vec.size())
        {
            return vec;    // Return the original vector
        }

        ct::vec_impl::vec_shape_t vec_info = vec.vector_shape();

        ct::vec_impl::vec_instr_queue_t& queue =
            CT_ACCESS_SINGLETON(ct::vec_impl::vec_instr_queue);
        queue.dispatch(vec_info.shape_id);

        ck::future<std::vector<double>> fval;
        CProxy_get_partial_vec_future vec_proxy =
            CProxy_get_partial_vec_future::ckNew(fval, k);

        std::size_t& vec_sdag_idx = queue.sdag_idx(vec_info.shape_id);

        CProxy_vector_impl dispatch_proxy = vec_info.proxy;
        dispatch_proxy.get_min_chunks(vec_sdag_idx, vec_info.vector_id,
            static_cast<int>(k), static_cast<int>(vec.size()), vec_proxy);

        ++vec_sdag_idx;

        std::vector<double> chunk_mins = fval.get();

        std::shared_ptr<ct::from_vector_generator> gen =
            std::make_shared<ct::from_vector_generator>(chunk_mins);
        ct::vector result{k, gen};

        return result;
    }

    template <typename Operand>
    auto unary_expr(
        Operand const& operand, std::shared_ptr<unary_operator> unary_op)
    {
        if constexpr (ct::traits::is_vec_type_v<
                          std::decay_t<Operand>>)
        {
            return ct::vec_impl::vec_expression<Operand, Operand>{operand,
                operand.size(), ct::util::Operation::unary_expr, unary_op};
        }
        else
        {
            return ct::mat_impl::mat_expression<Operand, Operand>{operand,
                operand.rows(), operand.cols(), ct::util::Operation::unary_expr,
                unary_op};
        }
    }

    template <typename Operand>
    auto custom_expr(
        Operand const& operand, std::shared_ptr<custom_operator> custom_op)
    {
        if constexpr (ct::traits::is_vec_type_v<std::decay_t<Operand>>)
        {
            return ct::vec_impl::vec_expression<Operand, Operand>{operand,
                operand.size(), ct::util::Operation::custom_expr, custom_op};
        }
        else
        {
            return ct::mat_impl::mat_expression<Operand, Operand>{operand,
                operand.rows(), operand.cols(),
                ct::util::Operation::custom_expr, custom_op};
        }
    }

    template <typename LHS, typename RHS, typename THS>
    auto where(LHS const& lhs, RHS const& rhs, THS const& ths)
    {
        if constexpr (ct::traits::is_ter_vec_type_v<LHS, RHS, THS>)
        {
            return ct::vec_impl::ter_vec_expression<LHS, RHS, THS>{
                lhs, rhs, ths, lhs.size(), ct::util::Operation::where};
        }
        else
        {
            return ct::mat_impl::ter_mat_expression<LHS, RHS, THS>{lhs, rhs,
                ths, lhs.rows(), lhs.cols(), ct::util::Operation::where};
        }
    }

    template <typename LHS, typename RHS>
    auto binary_expr(LHS const& lhs, RHS const& rhs,
        std::shared_ptr<binary_operator> binary_op)
    {
        if constexpr (ct::traits::is_bin_vec_type_v<LHS, RHS>)
        {
            return ct::vec_impl::vec_expression<LHS, RHS>{lhs, rhs, lhs.size(),
                ct::util::Operation::binary_expr, binary_op};
        }
        else
        {
            return ct::mat_impl::mat_expression<LHS, RHS>{lhs, rhs, lhs.rows(),
                lhs.cols(), ct::util::Operation::binary_expr, binary_op};
        }
    }

    // Helper function to create ct::vector from std::vector<double>
    inline ct::vector from_vector(const std::vector<double>& data)
    {
        return ct::vector(
            data.size(), std::make_shared<from_vector_generator>(data));
    }

    inline ct::matrix from_matrix(const std::vector<std::vector<double>>& data)
    {
        return ct::matrix(data.size(), data[0].size(),
            std::make_shared<from_matrix_generator>(data));
    }

    inline ct::vector from_vector(const double* data, uint64_t size)
    {
        return ct::vector(
            size, std::make_shared<from_vector_generator>(data, size));
    }

    inline ct::matrix from_matrix(
        const double* data, uint64_t rows, uint64_t cols)
    {
        return ct::matrix(rows, cols,
            std::make_shared<from_matrix_generator>(data, rows, cols));
    }

}    // namespace ct

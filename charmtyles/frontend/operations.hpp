#pragma once

#include <charmtyles/frontend/scalar.hpp>
#include <charmtyles/frontend/vector.hpp>

#include <type_traits>

namespace ct {

    namespace traits {
        template <typename... Ts>
        struct is_vec_type_impl<ct::vec_impl::vec_expression<Ts...>>
        {
            constexpr static bool value = true;
        };

        template <typename... Ts>
        struct is_mat_type_impl<ct::mat_impl::mat_expression<Ts...>>
        {
            constexpr static bool value = true;
        };

        template <typename LHS, typename RHS>
        struct is_vec_type
        {
            constexpr static bool value =
                is_vec_type_impl<typename std::decay<LHS>::type>::value &&
                is_vec_type_impl<typename std::decay<RHS>::type>::value;
        };

        template <typename LHS, typename RHS>
        struct is_mat_type
        {
            constexpr static bool value =
                is_mat_type_impl<typename std::decay<LHS>::type>::value &&
                is_mat_type_impl<typename std::decay<RHS>::type>::value;
        };

    }    // namespace traits

    template <typename LHS, typename RHS>
    auto operator+(LHS const& lhs, RHS const& rhs)
    {
        if constexpr (ct::traits::is_vec_type<LHS, RHS>::value)
        {
            return ct::vec_impl::vec_expression<LHS, RHS>{
                lhs, rhs, lhs.size(), ct::util::Operation::add};
        }
        else
        {
            return ct::mat_impl::mat_expression<LHS, RHS>{
                lhs, rhs, lhs.rows(), lhs.cols(), ct::util::Operation::add};
        }
    }

    template <typename LHS, typename RHS>
    auto operator-(LHS const& lhs, RHS const& rhs)
    {
        if constexpr (ct::traits::is_vec_type<LHS, RHS>::value)
        {
            return ct::vec_impl::vec_expression<LHS, RHS>{
                lhs, rhs, lhs.size(), ct::util::Operation::sub};
        }
        else
        {
            return ct::mat_impl::mat_expression<LHS, RHS>{
                lhs, rhs, lhs.rows(), lhs.cols(), ct::util::Operation::sub};
        }
    }

    template <typename LHS, typename RHS>
    auto operator/(LHS const& lhs, RHS const& rhs)
    {
        if constexpr (ct::traits::is_vec_type<LHS, RHS>::value)
        {
            return ct::vec_impl::vec_expression<LHS, RHS>{
                lhs, rhs, lhs.size(), ct::util::Operation::divide};
        }
        else
        {
            return ct::mat_impl::mat_expression<LHS, RHS>{
                lhs, rhs, lhs.rows(), lhs.cols(), ct::util::Operation::divide};
        }
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

    // BLAS L1: AXPY
    namespace blas_impl {

        class vec_axpy_expr
        {
            friend class ct::vector;

        public:
            vec_axpy_expr(double a_, ct::vector const& x_, ct::vector const& y_)
              : a(a_)
              , x(x_)
              , y(y_)
            {
            }

            std::size_t size() const
            {
                return x.size();
            }

        private:
            double a;
            ct::vector const& x;
            ct::vector const& y;
        };
    }    // namespace blas_impl

    inline vector::vector(blas_impl::vec_axpy_expr const& expr)
      : size_(expr.size())
      , vector_shape_(ct::vec_impl::get_vector_shape(size_))
      , node_(vector_shape_.vector_id, ct::util::Operation::axpy, expr.a, size_,
            expr.x.vector_shape().vector_id, expr.y.vector_shape().vector_id)
    {
        ct::vec_impl::vec_instr_queue_t& queue =
            CT_ACCESS_SINGLETON(ct::vec_impl::vec_instr_queue);

        queue.insert(node_, vector_shape_.shape_id);
    }

    inline vector& vector::operator=(blas_impl::vec_axpy_expr const& expr)
    {
        ct::vec_impl::vec_node node{vector_shape_.vector_id,
            ct::util::Operation::axpy, expr.a, size_,
            expr.x.vector_shape_.vector_id, expr.y.vector_shape_.vector_id};

        ct::vec_impl::vec_instr_queue_t& queue =
            CT_ACCESS_SINGLETON(ct::vec_impl::vec_instr_queue);

        queue.insert(node, vector_shape_.shape_id);

        return *this;
    }

    inline blas_impl::vec_axpy_expr axpy(
        double a, ct::vector const& x, ct::vector const& y)
    {
        std::size_t x_len = x.size();
        std::size_t y_len = y.size();
        CkAssert(x_len == y_len &&
            "Invalid vector dimensions passed to a*x + y Blas operation.");

        return ct::blas_impl::vec_axpy_expr{a, x, y};
    }

    // Non-implemented axpy variants
    inline void axpy(double a, ct::vector&& x, ct::vector const& y)
    {
        CkAbort("AXPY with rvalue reference parameter is not supported.");
    }

    inline void axpy(double a, ct::vector const& x, ct::vector&& y)
    {
        CkAbort("AXPY with rvalue reference parameter is not supported.");
    }

    inline void axpy(double a, ct::vector&& x, ct::vector&& y)
    {
        CkAbort("AXPY with rvalue reference parameter is not supported.");
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

    inline ct::mat_mul_impl::mat_mul_expr operator*(
        matrix const& lhs, matrix const& rhs)
    {
        return mat_mul_impl::mat_mul_expr(lhs, rhs);
    }

    inline mat_mul_impl::mat_mul_expr operator*(matrix const& lhs, matrix&& rhs)
    {
        CkAbort(
            "Matrix Multiplication not implemented for complex operations.");
    }

    inline mat_mul_impl::mat_mul_expr operator*(matrix&& lhs, matrix const& rhs)
    {
        CkAbort(
            "Matrix Multiplication not implemented for complex operations.");
    }

    inline mat_mul_impl::mat_mul_expr operator*(matrix&& lhs, matrix&& rhs)
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

    inline void unary_expr(
        ct::vector const& vec, std::shared_ptr<unary_operator> unary_operator)
    {
        ct::vec_impl::vec_shape_t vector_shape = vec.vector_shape();
        ct::vec_impl::vec_node root{vector_shape.vector_id,
            ct::util::Operation::unary_expr, unary_operator, vec.size()};
    }

    inline void unary_expr(
        ct::matrix const& mat, std::shared_ptr<unary_operator> unary_operator)
    {
        ct::mat_impl::mat_shape_t mat_shape = mat.matrix_shape();
        ct::mat_impl::mat_node root{mat_shape.matrix_id,
            ct::util::Operation::unary_expr, unary_operator, mat.rows(),
            mat.cols()};
    }
}    // namespace ct

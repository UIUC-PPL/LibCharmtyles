#pragma once

#include <charmtyles/frontend/scalar.hpp>
#include <charmtyles/frontend/vector.hpp>

#include <type_traits>
#include <stdexcept>

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

        template <typename T>
        struct is_unary_vec_type
        {
            constexpr static bool value = std::is_same_v<T, ct::vector>;
        };

        template <typename T>
        struct is_unary_mat_type
        {
            constexpr static bool value = std::is_same_v<T, ct::matrix>;
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

    
    inline ct::vector get_avg(ct::vector const& vec, std::size_t k)
    {
        if (k == 0) {
            throw std::invalid_argument("k must be greater than 0");
        }
        if (k >= vec.size()) {
            
            return vec;  // Return the original vector
        }
        
        ct::vec_impl::vec_shape_t vec_info = vec.vector_shape();

        ct::vec_impl::vec_instr_queue_t& queue =
            CT_ACCESS_SINGLETON(ct::vec_impl::vec_instr_queue);
        queue.dispatch(vec_info.shape_id);

        ck::future<std::vector<double>> fval;
        CProxy_get_partial_vec_future vec_proxy = CProxy_get_partial_vec_future::ckNew(fval, k);
        
        std::size_t& vec_sdag_idx = queue.sdag_idx(vec_info.shape_id);

        CProxy_vector_impl dispatch_proxy = vec_info.proxy;
        dispatch_proxy.get_avg_chunks(vec_sdag_idx, vec_info.vector_id, static_cast<int>(k), static_cast<int>(vec.size()), vec_proxy);

        ++vec_sdag_idx;

        std::vector<double> chunk_avgs = fval.get();
        
         
        std::shared_ptr<ct::data_generator> gen = std::make_shared<ct::data_generator>(chunk_avgs);
        ct::vector result{k, gen};
        
        return result;
    }

    
    inline ct::vector get_max(ct::vector const& vec, std::size_t k)
    {
        if (k == 0) {
            throw std::invalid_argument("k must be greater than 0");
        }
        if (k >= vec.size()) {
            
            return vec;  // Return the original vector
        }
        
        ct::vec_impl::vec_shape_t vec_info = vec.vector_shape();

        ct::vec_impl::vec_instr_queue_t& queue =
            CT_ACCESS_SINGLETON(ct::vec_impl::vec_instr_queue);
        queue.dispatch(vec_info.shape_id);

        ck::future<std::vector<double>> fval;
        CProxy_get_partial_vec_future vec_proxy = CProxy_get_partial_vec_future::ckNew(fval, k);
        
        std::size_t& vec_sdag_idx = queue.sdag_idx(vec_info.shape_id);

        CProxy_vector_impl dispatch_proxy = vec_info.proxy;
        dispatch_proxy.get_max_chunks(vec_sdag_idx, vec_info.vector_id, static_cast<int>(k), static_cast<int>(vec.size()), vec_proxy);

        ++vec_sdag_idx;

        std::vector<double> chunk_maxs = fval.get();
        
          
        std::shared_ptr<ct::data_generator> gen = std::make_shared<ct::data_generator>(chunk_maxs);
        ct::vector result{k, gen};
        
        return result;
    }

    
    inline ct::vector get_min(ct::vector const& vec, std::size_t k)
    {
        if (k == 0) {
            throw std::invalid_argument("k must be greater than 0");
        }
        if (k >= vec.size()) {
            
            return vec;  // Return the original vector
        }
        
        ct::vec_impl::vec_shape_t vec_info = vec.vector_shape();

        ct::vec_impl::vec_instr_queue_t& queue =
            CT_ACCESS_SINGLETON(ct::vec_impl::vec_instr_queue);
        queue.dispatch(vec_info.shape_id);

        ck::future<std::vector<double>> fval;
        CProxy_get_partial_vec_future vec_proxy = CProxy_get_partial_vec_future::ckNew(fval, k);
        
        std::size_t& vec_sdag_idx = queue.sdag_idx(vec_info.shape_id);

        CProxy_vector_impl dispatch_proxy = vec_info.proxy;
        dispatch_proxy.get_min_chunks(vec_sdag_idx, vec_info.vector_id, static_cast<int>(k), static_cast<int>(vec.size()), vec_proxy);

        ++vec_sdag_idx;

        std::vector<double> chunk_mins = fval.get();
        
          
        std::shared_ptr<ct::data_generator> gen = std::make_shared<ct::data_generator>(chunk_mins);
        ct::vector result{k, gen};
        
        return result;
    }

    namespace unary_impl {
        template <typename Operand>
        class unary_expression
        {
        public:
            explicit unary_expression(Operand const& operand_,
                std::shared_ptr<unary_operator> unary_op_)
              : operand(operand_)
              , unary_op(unary_op_)
            {
            }

            // Single operator() that uses traits
            auto operator()() const
            {
                if constexpr (ct::traits::is_unary_vec_type<Operand>::value)
                {
                    return create_vec_ast();
                }
                else if constexpr (ct::traits::is_unary_mat_type<
                                       Operand>::value)
                {
                    return create_mat_ast();
                }
                else
                {
                    static_assert(
                        ct::traits::is_unary_vec_type<Operand>::value ||
                            ct::traits::is_unary_mat_type<Operand>::value,
                        "Operand must be ct::vector or ct::matrix");
                }
            }

            std::size_t size() const
            {
                return operand.size();
            }
            std::size_t rows() const
            {
                return operand.rows();
            }
            std::size_t cols() const
            {
                return operand.cols();
            }

        private:
            // Vector AST creation
            std::vector<ct::vec_impl::vec_node> create_vec_ast() const
            {
                std::vector<ct::vec_impl::vec_node> operand_ast = operand();
                ct::vec_impl::vec_node& operand_root = operand_ast.front();

                ct::vec_impl::vec_node unary_node{operand_root.name_,
                    ct::util::Operation::unary_expr, unary_op,
                    operand_root.vec_len_};

                unary_node.left_ = 1;

                std::vector<ct::vec_impl::vec_node> ast;
                ast.reserve(operand_ast.size() + 1);

                ast.emplace_back(unary_node);
                std::copy(operand_ast.begin(), operand_ast.end(),
                    std::back_inserter(ast));

                for (int i = 1; i != ast.size(); ++i)
                {
                    if (ast[i].left_ != static_cast<std::size_t>(-1))
                        ast[i].left_ += 1;
                    if (ast[i].right_ != static_cast<std::size_t>(-1))
                        ast[i].right_ += 1;
                }

                return ast;
            }

            // Matrix AST creation
            std::vector<ct::mat_impl::mat_node> create_mat_ast() const
            {
                std::vector<ct::mat_impl::mat_node> operand_ast = operand();
                ct::mat_impl::mat_node& operand_root = operand_ast.front();

                ct::mat_impl::mat_node unary_node{operand_root.name_,
                    ct::util::Operation::unary_expr, unary_op,
                    operand_root.mat_row_len_, operand_root.mat_col_len_};

                unary_node.left_ = 1;

                std::vector<ct::mat_impl::mat_node> ast;
                ast.reserve(operand_ast.size() + 1);

                ast.emplace_back(unary_node);
                std::copy(operand_ast.begin(), operand_ast.end(),
                    std::back_inserter(ast));

                for (int i = 1; i != ast.size(); ++i)
                {
                    if (ast[i].left_ != static_cast<std::size_t>(-1))
                        ast[i].left_ += 1;
                    if (ast[i].right_ != static_cast<std::size_t>(-1))
                        ast[i].right_ += 1;
                }

                return ast;
            }

            Operand const& operand;
            std::shared_ptr<unary_operator> unary_op;
        };
    }    // namespace unary_impl

    // Vector constructors and assignment operators
    template <typename Operand>
    vector::vector(ct::unary_impl::unary_expression<Operand> const& e)
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

    template <typename Operand>
    vector& vector::operator=(
        ct::unary_impl::unary_expression<Operand> const& e)
    {
        std::vector<ct::vec_impl::vec_node> instr = e();
        ct::vec_impl::vec_node& root = instr.front();

        root.name_ = vector_shape_.vector_id;

        ct::vec_impl::vec_instr_queue_t& queue =
            CT_ACCESS_SINGLETON(ct::vec_impl::vec_instr_queue);

        queue.insert(instr, vector_shape_.shape_id);

        return *this;
    }

    // Matrix constructors and assignment operators
    template <typename Operand>
    matrix::matrix(ct::unary_impl::unary_expression<Operand> const& e)
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

    template <typename Operand>
    matrix& matrix::operator=(
        ct::unary_impl::unary_expression<Operand> const& e)
    {
        std::vector<ct::mat_impl::mat_node> instr = e();
        ct::mat_impl::mat_node& root = instr.front();

        root.name_ = matrix_shape_.matrix_id;

        ct::mat_impl::mat_instr_queue_t& queue =
            CT_ACCESS_SINGLETON(ct::mat_impl::mat_instr_queue);

        queue.insert(instr, matrix_shape_.shape_id);

        return *this;
    }    // namespace ct

    template <typename Operand>
    auto unary_expr(Operand& operand, std::shared_ptr<unary_operator> unary_op)
    {
        return ct::unary_impl::unary_expression<Operand>(operand, unary_op);
    }

    // Binary expression constructors and assignment operators for vectors
    template <typename LeftOperand, typename RightOperand>
    vector::vector(ct::binary_impl::binary_expression<LeftOperand, RightOperand> const& e)
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

    template <typename LeftOperand, typename RightOperand>
    vector& vector::operator=(
        ct::binary_impl::binary_expression<LeftOperand, RightOperand> const& e)
    {
        std::vector<ct::vec_impl::vec_node> instr = e();
        ct::vec_impl::vec_node& root = instr.front();

        root.name_ = vector_shape_.vector_id;

        ct::vec_impl::vec_instr_queue_t& queue =
            CT_ACCESS_SINGLETON(ct::vec_impl::vec_instr_queue);

        queue.insert(instr, vector_shape_.shape_id);

        return *this;
    }

    // Binary expression constructors and assignment operators for matrices
    template <typename LeftOperand, typename RightOperand>
    matrix::matrix(ct::binary_impl::binary_expression<LeftOperand, RightOperand> const& e)
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

    template <typename LeftOperand, typename RightOperand>
    matrix& matrix::operator=(
        ct::binary_impl::binary_expression<LeftOperand, RightOperand> const& e)
    {
        std::vector<ct::mat_impl::mat_node> instr = e();
        ct::mat_impl::mat_node& root = instr.front();

        root.name_ = matrix_shape_.matrix_id;

        ct::mat_impl::mat_instr_queue_t& queue =
            CT_ACCESS_SINGLETON(ct::mat_impl::mat_instr_queue);

        queue.insert(instr, matrix_shape_.shape_id);

        return *this;
    }

    // Global binary_expr function
    template <typename LeftOperand, typename RightOperand>
    auto binary_expr(LeftOperand& left_operand, RightOperand& right_operand, 
                     std::shared_ptr<binary_operator> binary_op)
    {
        return ct::binary_impl::binary_expression<LeftOperand, RightOperand>(
            left_operand, right_operand, binary_op);
    }

    namespace binary_impl {
    template <typename LeftOperand, typename RightOperand>
    class binary_expression
        {
        public:
            explicit binary_expression(LeftOperand const& left_operand_,
                                    RightOperand const& right_operand_,
                                    std::shared_ptr<binary_operator> binary_op_)
            : left_operand(left_operand_)
            , right_operand(right_operand_)
            , binary_op(binary_op_)
            {
            }

            // Create AST 
            auto operator()() const
            {
                if constexpr (ct::traits::is_unary_vec_type<LeftOperand>::value)
                {
                    return create_vec_ast();
                }
                else if constexpr (ct::traits::is_unary_mat_type<LeftOperand>::value)
                {
                    return create_mat_ast();
                }
            }

            std::size_t size() const { return left_operand.size(); }
            std::size_t rows() const { return left_operand.rows(); }
            std::size_t cols() const { return left_operand.cols(); }

        private:
            // Vector AST 
            std::vector<ct::vec_impl::vec_node> create_vec_ast() const
            {
                
                std::vector<ct::vec_impl::vec_node> left_ast = left_operand();
                std::vector<ct::vec_impl::vec_node> right_ast = right_operand();
                
                ct::vec_impl::vec_node& left_root = left_ast.front();
                ct::vec_impl::vec_node& right_root = right_ast.front();

                
                ct::vec_impl::vec_node binary_node{left_root.name_,
                    ct::util::Operation::binary_expr, binary_op,
                    left_root.vec_len_};

                
                binary_node.left_ = 1; 
                binary_node.right_ = 1 + left_ast.size(); 

                
                std::vector<ct::vec_impl::vec_node> ast;
                ast.reserve(1 + left_ast.size() + right_ast.size());
                
                ast.emplace_back(binary_node);
                std::copy(left_ast.begin(), left_ast.end(), std::back_inserter(ast));
                std::copy(right_ast.begin(), right_ast.end(), std::back_inserter(ast));

                
                for (int i = 1; i != 1 + left_ast.size(); ++i)
                {
                    if (ast[i].left_ != static_cast<std::size_t>(-1))
                        ast[i].left_ += 1;
                    if (ast[i].right_ != static_cast<std::size_t>(-1))
                        ast[i].right_ += 1;
                }

                
                for (int i = 1 + left_ast.size(); i != ast.size(); ++i)
                {
                    if (ast[i].left_ != static_cast<std::size_t>(-1))
                        ast[i].left_ += 1 + left_ast.size();
                    if (ast[i].right_ != static_cast<std::size_t>(-1))
                        ast[i].right_ += 1 + left_ast.size();
                }

                return ast;
            }

            // Matrix AST 
            std::vector<ct::mat_impl::mat_node> create_mat_ast() const
            {
               
                std::vector<ct::mat_impl::mat_node> left_ast = left_operand();
                std::vector<ct::mat_impl::mat_node> right_ast = right_operand();
                
                ct::mat_impl::mat_node& left_root = left_ast.front();
                ct::mat_impl::mat_node& right_root = right_ast.front();

                
                ct::mat_impl::mat_node binary_node{left_root.name_,
                    ct::util::Operation::binary_expr, binary_op,
                    left_root.mat_row_len_, left_root.mat_col_len_};

                
                binary_node.left_ = 1;  
                binary_node.right_ = 1 + left_ast.size();  

                
                std::vector<ct::mat_impl::mat_node> ast;
                ast.reserve(1 + left_ast.size() + right_ast.size());
                
                ast.emplace_back(binary_node);
                std::copy(left_ast.begin(), left_ast.end(), std::back_inserter(ast));
                std::copy(right_ast.begin(), right_ast.end(), std::back_inserter(ast));

                
                for (int i = 1; i != 1 + left_ast.size(); ++i)
                {
                    if (ast[i].left_ != static_cast<std::size_t>(-1))
                        ast[i].left_ += 1;
                    if (ast[i].right_ != static_cast<std::size_t>(-1))
                        ast[i].right_ += 1;
                }

                
                for (int i = 1 + left_ast.size(); i != ast.size(); ++i)
                {
                    if (ast[i].left_ != static_cast<std::size_t>(-1))
                        ast[i].left_ += 1 + left_ast.size();
                    if (ast[i].right_ != static_cast<std::size_t>(-1))
                        ast[i].right_ += 1 + left_ast.size();
                }

                return ast;
            }

            LeftOperand const& left_operand;
            RightOperand const& right_operand;
            std::shared_ptr<binary_operator> binary_op;
        };
    }

    // Helper function to create ct::vector from std::vector<double>
    inline ct::vector from_vector(const std::vector<double>& data)
    {
        return ct::vector(data.size(), std::make_shared<data_generator>(data));
    }

    namespace lu_impl
    {
        class lu_l_expr
        {
            friend class ct::matrix;

        public:
            lu_l_expr(ct::matrix const& source_)
              : source(source_)
            {
            }

            std::size_t rows() const
            {
                return source.rows();
            }

            std::size_t cols() const
            {
                return source.rows();
            }

        private:
            ct::matrix const& source;
        };

        class lu_u_expr
        {
            friend class ct::matrix;

        public:
            lu_u_expr(ct::matrix const& source_)
              : source(source_)
            {
            }

            std::size_t rows() const
            {
                return source.rows();
            }

            std::size_t cols() const
            {
                return source.cols();
            }

        private:
            ct::matrix const& source;
        };

        class lu_p_expr
        {
            friend class ct::matrix;

        public:
            lu_p_expr(ct::matrix const& source_)
              : source(source_)
            {
            }

            std::size_t rows() const
            {
                return source.rows();
            }

            std::size_t cols() const
            {
                return source.rows();
            }

        private:
            ct::matrix const& source;
        };
    }

    inline matrix::matrix(ct::lu_impl::lu_l_expr const& expr)
    {
        // Copy the shape
        row_size_ = expr.rows();
        col_size_ = expr.cols();
        matrix_shape_ = ct::mat_impl::get_mat_shape(row_size_, col_size_);
        
        // Create a node for LU L 
        node_ = ct::mat_impl::mat_node(matrix_shape_.matrix_id, ct::util::Operation::lu_l, 
                                     expr.source.node_);
                                     
        // Queue the operation
        ct::mat_impl::mat_instr_queue_t& queue = 
            CT_ACCESS_SINGLETON(ct::mat_impl::mat_instr_queue);
        queue.insert({node_}, matrix_shape_.shape_id);
    }

    inline matrix::matrix(ct::lu_impl::lu_u_expr const& expr)
    {
        // Copy the shape
        row_size_ = expr.rows();
        col_size_ = expr.cols(); 
        matrix_shape_ = ct::mat_impl::get_mat_shape(row_size_, col_size_);
        
        // Create a node for LU U 
        node_ = ct::mat_impl::mat_node(matrix_shape_.matrix_id, ct::util::Operation::lu_u, 
                                     expr.source.node_);
                                     
        // Queue the operation
        ct::mat_impl::mat_instr_queue_t& queue = 
            CT_ACCESS_SINGLETON(ct::mat_impl::mat_instr_queue);
        queue.insert({node_}, matrix_shape_.shape_id);
    }

    inline matrix::matrix(ct::lu_impl::lu_p_expr const& expr)
    {
        // Copy the shape
        row_size_ = expr.rows();
        col_size_ = expr.cols();
        matrix_shape_ = ct::mat_impl::get_mat_shape(row_size_, col_size_);
        
        // Create a node for LU P 
        node_ = ct::mat_impl::mat_node(matrix_shape_.matrix_id, ct::util::Operation::lu_p, 
                                     expr.source.node_);
                                     
        // Queue the operation
        ct::mat_impl::mat_instr_queue_t& queue = 
            CT_ACCESS_SINGLETON(ct::mat_impl::mat_instr_queue);
        queue.insert({node_}, matrix_shape_.shape_id);
    }

    inline ct::lu_impl::lu_l_expr get_L(matrix const& a)
    {
        return lu_impl::lu_l_expr(a);
    }

    inline ct::lu_impl::lu_u_expr get_U(matrix const& a)
    {
        return lu_impl::lu_u_expr(a);
    }

    inline ct::lu_impl::lu_p_expr get_P(matrix const& a)
    {
        return lu_impl::lu_p_expr(a);
    }

}    // namespace ct

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
            dot_expression(ct::vector const& lhs_, ct::matrix const& rhs_)
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
        };
    }    // namespace dot_impl

    vector::vector(dot_impl::dot_expression const& expr)
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
        return dot(rhs, lhs);
    }

    // Non-implemented dot product types
    void dot(ct::vector const& lhs, ct::matrix&& rhs)
    {
        CkAbort(
            "Dot Product with rvalue reference parameter is not supported.");
    }

    void dot(ct::vector&& lhs, ct::matrix&& rhs)
    {
        CkAbort(
            "Dot Product with rvalue reference parameter is not supported.");
    }

    void dot(ct::vector&& lhs, ct::matrix const& rhs)
    {
        CkAbort(
            "Dot Product with rvalue reference parameter is not supported.");
    }

    void dot(ct::matrix const& lhs, ct::vector&& rhs)
    {
        CkAbort(
            "Dot Product with rvalue reference parameter is not supported.");
    }

    void dot(ct::matrix&& lhs, ct::vector&& rhs)
    {
        CkAbort(
            "Dot Product with rvalue reference parameter is not supported.");
    }

    void dot(ct::matrix&& lhs, ct::vector const& rhs)
    {
        CkAbort(
            "Dot Product with rvalue reference parameter is not supported.");
    }

    // inline ct::vector dot(ct::vector const& lhs, ct::matrix const& rhs)
    // {
    //     std::size_t lhs_len = lhs.size();
    //     std::size_t rhs_cols = rhs.cols();
    //     CkAssert(lhs_len == rhs_cols && "Invalid dot product dimensions.");

    //     // Dispatch previous instructions belonging to this shape
    //     ct::vec_impl::vec_instr_queue_t& vec_queue =
    //         CT_ACCESS_SINGLETON(ct::vec_impl::vec_instr_queue);
    //     vec_queue.dispatch(lhs.vector_shape().shape_id);

    //     ct::mat_impl::mat_instr_queue_t& mat_queue =
    //         CT_ACCESS_SINGLETON(ct::mat_impl::mat_instr_queue);
    //     mat_queue.dispatch(rhs.matrix_shape().shape_id);

    //     ct::vector result{rhs.rows()};
    //     vec_queue.dispatch(result.vector_shape().shape_id);

    //     std::size_t& lhs_sdag_idx =
    //         vec_queue.sdag_idx(lhs.vector_shape().shape_id);
    //     std::size_t& rhs_sdag_idx =
    //         mat_queue.sdag_idx(rhs.matrix_shape().shape_id);

    //     std::size_t& result_sdag_idx =
    //         vec_queue.sdag_idx(result.vector_shape().shape_id);

    //     CProxy_matrix_impl dispatch_proxy = rhs.matrix_shape().proxy;
    //     dispatch_proxy.mat_vec_dot(rhs_sdag_idx, rhs.matrix_shape().matrix_id,
    //         result_sdag_idx, result.vector_shape().proxy);
    //     CProxy_vector_impl lhs_proxy = lhs.vector_shape().proxy;
    //     lhs_proxy.send_to_matrix(lhs_sdag_idx, lhs.vector_shape().vector_id,
    //         rhs_sdag_idx, dispatch_proxy);

    //     ++lhs_sdag_idx;
    //     ++rhs_sdag_idx;
    //     ++result_sdag_idx;

    //     return result;
    // }

    // inline ct::vector dot(ct::matrix const& lhs, ct::vector const& rhs)
    // {
    //     return dot(rhs, lhs);
    // }

}    // namespace ct

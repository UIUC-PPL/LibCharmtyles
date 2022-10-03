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

}    // namespace ct

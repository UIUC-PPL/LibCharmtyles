#pragma once

#include <charmtyles/frontend/vector.hpp>

#include <type_traits>

namespace ct {

    namespace traits {
        template <typename... Ts>
        struct is_vec_type_impl<ct::vec_impl::vec_expression<Ts...>>
        {
            using type = void;
        };

        template <typename LHS, typename RHS>
        struct is_vec_type
        {
            using type_LHS =
                typename is_vec_type_impl<typename std::decay<LHS>::type>::type;
            using type_RHS =
                typename is_vec_type_impl<typename std::decay<RHS>::type>::type;

            using type =
                std::enable_if_t<std::is_same<type_LHS, type_RHS>::value>;
        };
    }    // namespace traits

    template <typename LHS, typename RHS,
        typename = typename ct::traits::is_vec_type<LHS, RHS>::type>
    auto operator+(LHS const& lhs, RHS const& rhs)
    {
        return ct::vec_impl::vec_expression<LHS, RHS>{
            lhs, rhs, lhs.size(), ct::util::Operation::add};
    }

    template <typename LHS, typename RHS,
        typename = typename ct::traits::is_vec_type<LHS, RHS>::type>
    auto operator-(LHS const& lhs, RHS const& rhs)
    {
        return ct::vec_impl::vec_expression<LHS, RHS>{
            lhs, rhs, lhs.size(), ct::util::Operation::sub};
    }

}    // namespace ct

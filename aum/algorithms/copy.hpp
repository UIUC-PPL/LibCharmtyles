// Copyright (C) 2022 Nikunj Gupta
//
// This program is free software: you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
//  Software Foundation, version 3.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
// more details.
//
// You should have received a copy of the GNU General Public License along
// with this program. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <aum/frontend/matrix.hpp>
#include <aum/frontend/scalar.hpp>
#include <aum/frontend/vector.hpp>

namespace aum {

    template <typename DContainer, typename SContainer,
        typename = std::enable_if_t<
            std::is_same_v<std::decay_t<DContainer>, std::decay_t<SContainer>>>>
    void copy(DContainer&& dest, SContainer&& src)
    {
        if constexpr (std::is_rvalue_reference_v<decltype(src)>)
        {
            dest = src;
        }
        else
        {
            src.copy(dest);
        }
    }

    template <typename Container>
    auto copy(Container&& src)
    {
        if constexpr (std::is_rvalue_reference_v<decltype(src)>)
        {
            return std::forward<Container>(src);
        }
        else
        {
            return src.copy();
        }
    }

}    // namespace aum

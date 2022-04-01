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

namespace aum { namespace blas {

    // BLAS L2: y <- a * A * x + b * y
    aum::vector gemv(aum::scalar const& a, aum::matrix const& A,
        aum::vector const& x, aum::scalar const& b, aum::vector const& y)
    {
        //
    }

}}    // namespace aum::blas
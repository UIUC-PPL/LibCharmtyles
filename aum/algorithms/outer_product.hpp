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
#include <aum/frontend/vector.hpp>

namespace aum {

    aum::matrix outer(aum::vector const& v1, aum::vector const& v2)
    {
        assert((v1.size() == v2.size()) &&
            "Vectors provided with incompatible sizes");

        aum::matrix result{v1.size(), v1.size()};

        int w_tag = result.write_tag();
        v1.send_to_matrix_rows(w_tag, result);
        v2.send_to_matrix_rows_2(w_tag, result);
        result.proxy().outer(w_tag);
        result.update_tags();

        return result;
    }
}    // namespace aum

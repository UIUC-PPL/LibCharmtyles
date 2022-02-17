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

    aum::scalar reduce_add(aum::vector const& v)
    {
        aum::scalar result;
        int scalar_tag = result.write_tag();

        int read_tag = v.read_tag();
        v.proxy().reduce_add(read_tag, scalar_tag, result.proxy());
        v.inc_reads();

        result.update_tags();
        return result;
    }

}    // namespace aum

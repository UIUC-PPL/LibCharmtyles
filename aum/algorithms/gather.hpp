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

#include <aum/backend/container_base.hpp>

#include <aum/util/view.hpp>

namespace aum {

    aum::view<double, aum::matrix> gather(aum::matrix const& m)
    {
        CProxy_matrix_container proxy =
            CProxy_matrix_container::ckNew(m.rows(), m.cols());

        int r_tag = m.read_tag();
        m.proxy().gather_matrix(r_tag, proxy);
        m.inc_reads();

        ck::future<aum::view<double, aum::matrix>> f;
        proxy.get_underlying(1, f);

        return f.get();
    }

    aum::view<double, aum::vector> gather(aum::vector const& v)
    {
        CProxy_vector_container proxy =
            CProxy_vector_container::ckNew(v.size());

        int r_tag = v.read_tag();
        v.proxy().gather_vector(r_tag, proxy);
        v.inc_reads();

        ck::future<aum::view<double, aum::vector>> f;
        proxy.get_underlying(1, f);

        return f.get();
    }

}    // namespace aum
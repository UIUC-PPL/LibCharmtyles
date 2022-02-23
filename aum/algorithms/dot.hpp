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

    aum::scalar dot(aum::vector const& v1, aum::vector const& v2)
    {
        assert((v1.size() == v2.size()) &&
            "Vectors provided with incompatible sizes");

        // Are they the same set of vectors?
        if (v1.proxy() == v2.proxy())
        {
            aum::scalar result;
            int scalar_tag = result.write_tag();

            int read_tag = v1.read_tag();
            v1.proxy().self_dot(read_tag, scalar_tag, result.proxy());
            v1.inc_reads();

            result.update_tags();
            return result;
        }

        aum::scalar result;
        int scalar_tag = result.write_tag();

        int read_tag = v1.read_tag();
        v2.send_to_1(read_tag, v1);
        v1.proxy().dot(read_tag, scalar_tag, result.proxy());
        v1.inc_reads();

        result.update_tags();
        return result;
    }

    // v1 is temporary - work from v1 to keep v2 free
    aum::scalar dot(aum::vector&& v1, aum::vector const& v2)
    {
        assert((v1.size() == v2.size()) &&
            "Vectors provided with incompatible sizes");

        aum::scalar result;
        int scalar_tag = result.write_tag();

        int read_tag = v1.read_tag();
        v2.send_to_1(read_tag, v1);
        v1.proxy().dot(read_tag, scalar_tag, result.proxy());
        v1.inc_reads();

        result.update_tags();
        return result;
    }

    // v2 is temporary - work from v2 to keep v1 free
    aum::scalar dot(aum::vector const& v1, aum::vector&& v2)
    {
        assert((v1.size() == v2.size()) &&
            "Vectors provided with incompatible sizes");

        aum::scalar result;
        int scalar_tag = result.write_tag();

        int read_tag = v2.read_tag();
        v1.send_to_1(read_tag, v2);
        v2.proxy().dot(read_tag, scalar_tag, result.proxy());
        v2.inc_reads();

        result.update_tags();
        return result;
    }

    // v1 & v2 are temporary - work from v1
    aum::scalar dot(aum::vector&& v1, aum::vector&& v2)
    {
        assert((v1.size() == v2.size()) &&
            "Vectors provided with incompatible sizes");

        aum::scalar result;
        int scalar_tag = result.write_tag();

        int read_tag = v1.read_tag();
        v2.send_to_1(read_tag, v1);
        v1.proxy().dot(read_tag, scalar_tag, result.proxy());
        v1.inc_reads();

        result.update_tags();
        return result;
    }

    aum::vector dot(aum::matrix const& m1, aum::vector const& v1)
    {
        assert(m1.cols() == v1.size() &&
            "Incompatible matrix and vector dimensions");

        aum::vector result{m1.rows()};
        int vector_tag = result.write_tag();

        int read_tag = m1.read_tag();
        v1.send_for_matrix_vector_multiply(read_tag, m1);
        m1.proxy().matrix_vector_multiply(read_tag, vector_tag, result.proxy());
        m1.inc_reads();

        result.update_tags();
        return result;
    }

    aum::vector dot(aum::vector const& v1, aum::matrix const& m1)
    {
        assert(m1.rows() == v1.size() &&
            "Incompatible matrix and vector dimensions");

        aum::vector result{m1.cols()};
        int vector_tag = result.write_tag();

        int read_tag = m1.read_tag();
        v1.send_for_vector_matrix_multiply(read_tag, m1);
        m1.proxy().vector_matrix_multiply(read_tag, vector_tag, result.proxy());
        m1.inc_reads();

        result.update_tags();
        return result;
    }

}    // namespace aum

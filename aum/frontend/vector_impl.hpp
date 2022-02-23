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

    template <typename Container>
    void vector::send_to_1(int result_tag, Container&& result) const
    {
        ++write_tag_;

        proxy_.send_to_1(read_tag_, result_tag, result.proxy());
    }

    template <typename Container>
    void vector::send_to_2(int result_tag, Container&& result) const
    {
        ++write_tag_;

        proxy_.send_to_2(read_tag_, result_tag, result.proxy());
    }

    template <typename Container>
    void vector::send_for_matrix_vector_multiply(
        int result_tag, Container&& result) const
    {
        ++write_tag_;

        proxy_.send_for_matrix_vector_multiply(read_tag_, result_tag,
            result.num_chares_x(), result.num_chares_y(), result.proxy());
    }

    template <typename Container>
    void vector::send_for_vector_matrix_multiply(
        int result_tag, Container&& result) const
    {
        ++write_tag_;

        proxy_.send_for_vector_matrix_multiply(read_tag_, result_tag,
            result.num_chares_x(), result.num_chares_y(), result.proxy());
    }

    vector operator+(vector const& v1, vector const& v2)
    {
        assert((v1.size() == v2.size()) &&
            "Vectors provided with incompatible sizes");

        vector result{v1.size()};

        int w_tag = result.write_tag();
        v1.send_to_1(w_tag, result);
        v2.send_to_2(w_tag, result);
        result.proxy().add(w_tag);
        result.update_tags();

        return result;
    }

    vector operator+(vector&& v1, vector const& v2)
    {
        assert((v1.size() == v2.size()) &&
            "Vectors provided with incompatible sizes");

        int w_tag = v1.write_tag();
        v2.send_to_1(w_tag, v1);
        v1.proxy().plus_add(w_tag);
        v1.update_tags();

        return std::move(v1);
    }

    vector operator+(vector const& v1, vector&& v2)
    {
        assert((v1.size() == v2.size()) &&
            "Vectors provided with incompatible sizes");

        int w_tag = v2.write_tag();
        v1.send_to_1(w_tag, v2);
        v2.proxy().plus_add(w_tag);
        v2.update_tags();

        return std::move(v2);
    }

    vector operator+(vector&& v1, vector&& v2)
    {
        assert((v1.size() == v2.size()) &&
            "Vectors provided with incompatible sizes");

        int w_tag = v2.write_tag();
        v1.send_to_1(w_tag, v2);
        v2.proxy().plus_add(w_tag);
        v2.update_tags();

        return std::move(v2);
    }

    vector operator-(vector const& v1, vector const& v2)
    {
        assert((v1.size() == v2.size()) &&
            "Vectors provided with incompatible sizes");

        vector result{v1.size()};

        int w_tag = result.write_tag();
        v1.send_to_1(w_tag, result);
        v2.send_to_2(w_tag, result);
        result.proxy().subtract(w_tag);
        result.update_tags();

        return result;
    }

    vector operator-(vector&& v1, vector const& v2)
    {
        assert((v1.size() == v2.size()) &&
            "Vectors provided with incompatible sizes");

        int w_tag = v1.write_tag();
        v2.send_to_1(w_tag, v1);
        v1.proxy().minus_subtract(w_tag, false);
        v1.update_tags();

        return std::move(v1);
    }

    vector operator-(vector const& v1, vector&& v2)
    {
        assert((v1.size() == v2.size()) &&
            "Vectors provided with incompatible sizes");

        int w_tag = v2.write_tag();
        v1.send_to_1(w_tag, v2);
        v2.proxy().minus_subtract(w_tag, true);
        v2.update_tags();

        return std::move(v2);
    }

    vector operator-(vector&& v1, vector&& v2)
    {
        assert((v1.size() == v2.size()) &&
            "Vectors provided with incompatible sizes");

        int w_tag = v2.write_tag();
        v1.send_to_1(w_tag, v2);
        v2.proxy().minus_subtract(w_tag, true);
        v2.update_tags();

        return std::move(v2);
    }

    vector operator*(double value, vector const& v1)
    {
        vector result{v1.size()};

        int w_tag = result.write_tag();
        v1.send_to_1(w_tag, result);
        result.proxy().scalar_multiply(w_tag, value);
        result.update_tags();

        return result;
    }

    vector operator*(scalar const& s, vector const& v1)
    {
        vector result{v1.size()};

        int w_tag = result.write_tag();
        v1.send_to_1(w_tag, result);
        s.send_to_vector(w_tag, result);
        result.proxy().aum_scalar_multiply(w_tag);
        result.update_tags();

        return result;
    }

}    // namespace aum

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

#include <aum/frontend/scalar.hpp>

namespace aum {

    template <typename Container>
    void scalar::send_to_1(int result_tag, Container&& result) const
    {
        ++write_tag_;

        proxy_.send_to_1(read_tag_, result_tag, result.proxy());
    }

    template <typename Container>
    void scalar::send_to_2(int result_tag, Container&& result) const
    {
        ++write_tag_;

        proxy_.send_to_2(read_tag_, result_tag, result.proxy());
    }

    template <typename Container>
    void scalar::send_to_vector(int result_tag, Container&& result) const
    {
        ++write_tag_;

        proxy_.send_to_vector(read_tag_, result_tag, result.proxy());
    }

    template <typename Container>
    void scalar::send_to_matrix(int result_tag, Container&& result) const
    {
        ++write_tag_;

        proxy_.send_to_matrix(read_tag_, result_tag, result.proxy());
    }

    scalar operator+(scalar const& v1, double v2)
    {
        scalar result{v2};

        int w_tag = result.write_tag();
        v1.send_to_1(w_tag, result);
        result.proxy().plus_add(w_tag);
        result.update_tags();

        return result;
    }

    scalar operator+(scalar&& v1, double v2)
    {
        int w_tag = v1.write_tag();
        v1.proxy().add_double(w_tag, v2);
        v1.update_tags();

        return std::move(v1);
    }

    scalar operator+(double v2, scalar const& v1)
    {
        scalar result{v2};

        int w_tag = result.write_tag();
        v1.send_to_1(w_tag, result);
        result.proxy().plus_add(w_tag);
        result.update_tags();

        return result;
    }

    scalar operator+(double v2, scalar&& v1)
    {
        int w_tag = v1.write_tag();
        v1.proxy().add_double(w_tag, v2);
        v1.update_tags();

        return std::move(v1);
    }

    scalar operator+(scalar const& v1, scalar const& v2)
    {
        scalar result{};

        int w_tag = result.write_tag();
        v1.send_to_1(w_tag, result);
        v2.send_to_2(w_tag, result);
        result.proxy().add(w_tag);
        result.update_tags();

        return result;
    }

    scalar operator+(scalar&& v1, scalar const& v2)
    {
        int w_tag = v1.write_tag();
        v2.send_to_1(w_tag, v1);
        v1.proxy().plus_add(w_tag);
        v1.update_tags();

        return std::move(v1);
    }

    scalar operator+(scalar const& v1, scalar&& v2)
    {
        int w_tag = v2.write_tag();
        v1.send_to_1(w_tag, v2);
        v2.proxy().plus_add(w_tag);
        v2.update_tags();

        return std::move(v2);
    }

    scalar operator+(scalar&& v1, scalar&& v2)
    {
        int w_tag = v2.write_tag();
        v1.send_to_1(w_tag, v2);
        v2.proxy().plus_add(w_tag);
        v2.update_tags();

        return std::move(v2);
    }

    scalar operator-(scalar const& v1, double v2)
    {
        scalar result{v2};

        int w_tag = result.write_tag();
        v1.send_to_1(w_tag, result);
        result.proxy().minus_subtract(w_tag, true);
        result.update_tags();

        return result;
    }

    scalar operator-(double v2, scalar const& v1)
    {
        scalar result{v2};

        int w_tag = result.write_tag();
        v1.send_to_1(w_tag, result);
        result.proxy().minus_subtract(w_tag, false);
        result.update_tags();

        return result;
    }

    scalar operator-(scalar&& v1, double v2)
    {
        int w_tag = v1.write_tag();
        v1.proxy().subtract_double(w_tag, v2, false);
        v1.update_tags();

        return std::move(v1);
    }

    scalar operator-(double v2, scalar&& v1)
    {
        int w_tag = v1.write_tag();
        v1.proxy().subtract_double(w_tag, v2, true);
        v1.update_tags();

        return std::move(v1);
    }

    scalar operator-(scalar const& v1, scalar const& v2)
    {
        scalar result{};

        int w_tag = result.write_tag();
        v1.send_to_1(w_tag, result);
        v2.send_to_2(w_tag, result);
        result.proxy().subtract(w_tag);
        result.update_tags();

        return result;
    }

    scalar operator-(scalar&& v1, scalar const& v2)
    {
        int w_tag = v1.write_tag();
        v2.send_to_1(w_tag, v1);
        v1.proxy().minus_subtract(w_tag, false);
        v1.update_tags();

        return std::move(v1);
    }

    scalar operator-(scalar const& v1, scalar&& v2)
    {
        int w_tag = v2.write_tag();
        v1.send_to_1(w_tag, v2);
        v2.proxy().minus_subtract(w_tag, true);
        v2.update_tags();

        return std::move(v2);
    }

    scalar operator-(scalar&& v1, scalar&& v2)
    {
        int w_tag = v2.write_tag();
        v1.send_to_1(w_tag, v2);
        v2.proxy().minus_subtract(w_tag, true);
        v2.update_tags();

        return std::move(v2);
    }

    scalar operator*(scalar const& s1, double s2)
    {
        scalar result{s2};

        int w_tag = result.write_tag();
        s1.send_to_1(w_tag, result);
        result.proxy().multiply_immediate(w_tag);
        result.update_tags();

        return result;
    }

    scalar operator*(double s2, scalar const& s1)
    {
        scalar result{s2};

        int w_tag = result.write_tag();
        s1.send_to_1(w_tag, result);
        result.proxy().multiply_immediate(w_tag);
        result.update_tags();

        return result;
    }

    scalar operator*(scalar&& s1, double s2)
    {
        int w_tag = s1.write_tag();
        s1.proxy().multiply_double(w_tag, s2);
        s1.update_tags();

        return std::move(s1);
    }

    scalar operator*(double s2, scalar&& s1)
    {
        int w_tag = s1.write_tag();
        s1.proxy().multiply_double(w_tag, s2);
        s1.update_tags();

        return std::move(s1);
    }

    scalar operator*(scalar const& s1, scalar const& s2)
    {
        scalar result{};

        int w_tag = result.write_tag();
        s1.send_to_1(w_tag, result);
        s2.send_to_2(w_tag, result);
        result.proxy().multiply(w_tag);
        result.update_tags();

        return result;
    }

    scalar operator*(scalar&& s1, scalar const& s2)
    {
        int w_tag = s1.write_tag();
        s2.send_to_1(w_tag, s1);
        s1.proxy().multiply_immediate(w_tag);
        s1.update_tags();

        return std::move(s1);
    }

    scalar operator*(scalar const& s2, scalar&& s1)
    {
        int w_tag = s1.write_tag();
        s2.send_to_1(w_tag, s1);
        s1.proxy().multiply_immediate(w_tag);
        s1.update_tags();

        return std::move(s1);
    }

    scalar operator*(scalar&& s1, scalar&& s2)
    {
        int w_tag = s1.write_tag();
        s2.send_to_1(w_tag, s1);
        s1.proxy().multiply_immediate(w_tag);
        s1.update_tags();

        return std::move(s1);
    }

}    // namespace aum

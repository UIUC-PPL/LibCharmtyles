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

    // BLAS Level 1: y <- a*x + y
    aum::vector axpy(
        aum::scalar const& a, aum::vector const& x, aum::vector const& y)
    {
        assert((x.size() == y.size()) &&
            "Vectors provided with incompatible sizes");

        aum::vector result{x.size()};

        int w_tag = result.write_tag();

        a.send_to_vector(w_tag, result);
        x.send_to_1(w_tag, result);
        y.send_to_2(w_tag, result);
        result.proxy().axpy(w_tag, 1.);
        result.update_tags();

        return result;
    }

    aum::vector axpy(double multiplier, aum::scalar const& a,
        aum::vector const& x, aum::vector const& y)
    {
        assert((x.size() == y.size()) &&
            "Vectors provided with incompatible sizes");

        aum::vector result{x.size()};

        int w_tag = result.write_tag();

        a.send_to_vector(w_tag, result);
        x.send_to_1(w_tag, result);
        y.send_to_2(w_tag, result);
        result.proxy().axpy(w_tag, multiplier);
        result.update_tags();

        return result;
    }

    // BLAS Level 1: y <- a*x + y
    aum::vector axpy(double a, aum::vector const& x, aum::vector const& y)
    {
        assert((x.size() == y.size()) &&
            "Vectors provided with incompatible sizes");

        aum::vector result{x.size()};

        int w_tag = result.write_tag();

        x.send_to_1(w_tag, result);
        y.send_to_2(w_tag, result);
        result.proxy().axpy_scalar(w_tag, a);
        result.update_tags();

        return result;
    }

    // BLAS Level 1: Norm2: || x || <- sqrt(Sum (x_i ^ 2))
    aum::scalar squared_norm(aum::vector const& x)
    {
        aum::scalar result;
        int scalar_tag = result.write_tag();

        int read_tag = x.read_tag();
        x.proxy().norm(read_tag, scalar_tag, 2, result.proxy());
        x.inc_reads();

        result.update_tags();
        return result;
    }

    // BLAS Level 1: Normp: || x ||_p <- (Sum (x_i ^ p)) ^ (1 / p)
    aum::scalar norm_p(int p, aum::vector const& x)
    {
        aum::scalar result;
        int scalar_tag = result.write_tag();

        int read_tag = x.read_tag();
        x.proxy().norm(read_tag, scalar_tag, p, result.proxy());
        x.inc_reads();

        result.proxy().norm_p(scalar_tag, p);

        result.update_tags();
        return result;
    }

    // BLAS Level 1: Swap(v1, v2)
    template <typename Container>
    void swap(Container const& c1, Container const& c2)
    {
        Container temp = c1;
        c2 = c1;
        c1 = temp;
    }

    template <typename Container>
    aum::scalar max(Container const& c)
    {
        aum::scalar result;
        int scalar_tag = result.write_tag();

        int read_tag = c.read_tag();
        c.proxy().max(read_tag, scalar_tag, result.proxy());
        c.inc_reads();

        result.update_tags();
        return result;
    }

    template <typename Container>
    aum::scalar min(Container const& c)
    {
        aum::scalar result;
        int scalar_tag = result.write_tag();

        int read_tag = c.read_tag();
        c.proxy().min(read_tag, scalar_tag, result.proxy());
        c.inc_reads();

        result.update_tags();
        return result;
    }
}}    // namespace aum::blas
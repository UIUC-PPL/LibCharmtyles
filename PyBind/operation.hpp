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

#include <aum/aum.hpp>

namespace aum { namespace bind {

    enum class oper: std::size_t
    {
        add,
        sub,
        dot,
        outer,
        scalar_mul
    };

    template <typename Container1, typename Container2>
    class operation
    {
    public:
        operation(Container1 const& _c1, Container2 const& _c2)
          : c1(_c1)
          , c2(_c2)
        {
        }

        auto execute(oper const & op) const
        {
            switch (op)
            {
            case oper ::add: {
                auto res = c1 + c2;
                return res;
            }
            case oper ::sub: {
                auto res = c1 - c2;
                return res;
            }
            // case oper ::dot: {
            //     auto res = aum::dot(c1, c2);
            //     return res;
            // }
            // case oper ::outer: {
            //     auto res = aum::outer(c1, c2);
            //     return res;
            // }
            // case oper ::scalar_mul: {
            //     auto res = c1 * c2;
            //     return res;
            // }
            default:
                CmiAbort("Incorrect Operator.");
            }
        }

    private:
        Container1 const& c1;
        Container2 const& c2;
    };

    aum::vector make_vector(int size)
    {
        return aum::vector{size};
    }

    aum::vector make_vector(int size, aum::random)
    {
        return aum::vector{size, aum::random{}};
    }

    aum::vector make_vector(int size, double value)
    {
        return aum::vector{size, value};
    }

    aum::vector make_vector(int size, std::unique_ptr<aum::generator>&& gen)
    {
        return aum::vector{size, std::move(gen)};
    }

    aum::scalar make_scalar(double value)
    {
        return aum::scalar{value};
    }

    aum::matrix make_matrix(int rows, int cols)
    {
        return aum::matrix{rows, cols};
    }

    aum::matrix make_matrix(int rows, int cols, aum::random)
    {
        return aum::matrix{rows, cols, aum::random{}};
    }

    aum::matrix make_matrix(int rows, int cols, double value)
    {
        return aum::matrix{rows, cols, value};
    }

}}    // namespace aum::bind

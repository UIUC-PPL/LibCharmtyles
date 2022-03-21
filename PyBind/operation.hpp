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

    enum class operator: std::size_t
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

        auto execute(operator const & op) const
        {
            switch (op)
            {
            case operator ::add:
                auto res = c1 + c2;
                return res;
            case operator ::sub:
                auto res = c1 - c2;
                return res;
            case operator ::dot:
                auto res = aum::dot(c1, c2);
                return res;
            case operator ::outer:
                auto res = aum::outer(c1, c2);
                return res;
            case operator ::scalar_mul:
                auto res = c1 * c2;
                return res;
            default:
                CmiAbort("Incorrect Operator.");
            }
        }

    private:
        Container1 const& c1;
        Container2 const& c2;
    };

}}    // namespace aum::bind
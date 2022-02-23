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

#include "scalar_multiply.decl.h"

class Main : public CBase_Main
{
public:
    Main(CkArgMsg* msg)
    {
        double start = CkWallTimer();
        aum::matrix A{1010, 1050, 1.};
        aum::matrix B{1010, 1050, 2.};

        // No temporaries
        aum::scalar s{5.0};

        B = 5 * A;

        B = s * A;

        A = 5 * (A - B);

        aum::matrix C{1010, 1050, 1.};
        aum::vector a{1050, 1.};
        aum::vector v = aum::dot(C, a);
        aum::scalar val = aum::reduce_add(v);
        val.print_value("dot C, a");

        aum::vector b{1010, 1.};
        aum::vector y = aum::dot(b, C);
        val = aum::reduce_add(y);
        val.print_value("dot b, C");

        aum::wait_and_exit(val, start);
    }
};

#include "scalar_multiply.def.h"

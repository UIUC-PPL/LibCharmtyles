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

#include "vectors.decl.h"

class Main : public CBase_Main
{
public:
    Main(CkArgMsg* msg)
    {
        thisProxy.benchmark();
    }

    void benchmark()
    {
        double start = CkWallTimer();
        aum::vector A{1000000, 1.};
        aum::vector B{1000000, 2.};

        // No temporaries
        aum::scalar s = aum::dot(A, B);
        s.print_value();

        // 2 temporaries
        s = aum::dot(A + B, B - A);
        s.print_value();

        // left temporary
        s = aum::dot(A + B, A);
        s.print_value();

        // right temporary
        s = aum::dot(A, B - A);
        s.print_value();

        // Same dot
        s = aum::dot(A, A);

        B = 5 * A;

        B = s * A;

        A = 5 * (A - B);

        aum::wait_and_exit(A, start);
    }
};

#include "vectors.def.h"

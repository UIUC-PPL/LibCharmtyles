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

#include "multi_dot.decl.h"

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

        // Initialized condition
        aum::matrix A{3000, 3000, 1.};
        aum::vector p{3000, aum::random{}};

        aum::vector Ap = aum::dot(A, p);

        for (int i = 0; i != 100; ++i)
        {
            ckout << "Index: " << i << ", val: " << aum::reduce_add(Ap).get()
                  << endl;
            Ap = aum::dot(A, p);
        }

        aum::wait_and_exit(Ap, start);
    }
};

#include "multi_dot.def.h"

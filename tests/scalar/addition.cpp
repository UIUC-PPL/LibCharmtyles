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

#include "addition.decl.h"

class Main : public CBase_Main
{
public:
    Main(CkArgMsg* msg)
    {
        double start = CkWallTimer();
        aum::scalar A{1.1};
        aum::scalar B{2.2};
        aum::scalar C{3.3};
        aum::scalar D{4.4};

        // Force 2 temporaries
        aum::scalar E = (A + D) + (B + C);

        // 1 temp to the left
        A = B + C + D;

        // 1 temp to the right
        B = C + (A + D);

        B = 5 + C;

        B = C + 5;

        B.print_value();

        aum::wait_and_exit(A, start);
    }
};

#include "addition.def.h"

// Copyright (C) 2020 Nikunj Gupta
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

#include "addition.decl.h"

#include <aum/aum.hpp>

class Main : public CBase_Main
{
public:
    Main(CkArgMsg* msg)
    {
        aum::vector A{1000000, 1.1};
        aum::vector B{1000000, 2.2};
        aum::vector C{1000000, 3.3};
        aum::vector D{1000000, 4.4};

        // Force 2 temporaries
        aum::vector E = (A + D) + (B + C);

        // 1 temp to the left
        A = B + C + D;

        // 1 temp to the right
        B = C + (A + D);

        aum::synchronize(B);
    }
};

#include "addition.def.h"

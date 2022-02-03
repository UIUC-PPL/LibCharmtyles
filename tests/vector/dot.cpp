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

#include "dot.decl.h"

#include <aum/aum.hpp>

class Main : public CBase_Main
{
public:
    Main(CkArgMsg* msg)
    {
        double start = CkWallTimer();
        aum::vector A{1000000, 1.1};
        aum::vector B{1000000, 2.2};

        aum::scalar s = aum::dot(A, B);
        s.print_value();

        aum::wait_and_exit(s, start);
    }
};

#include "dot.def.h"

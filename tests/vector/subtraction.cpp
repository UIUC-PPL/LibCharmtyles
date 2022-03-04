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

#include "subtraction.decl.h"

class vec_gen : public aum::generator
{
public:
    vec_gen() = default;

    using aum::generator::generator;

    void generate(int index, int size, double* data)
    {
        for (int i = 0; i != size; ++i)
            data[i] = i + index;
    }

    PUPable_decl(vec_gen);
    vec_gen(CkMigrateMessage* m)
      : aum::generator(m)
    {
    }

    virtual void pup(PUP::er& p)
    {
        aum::generator::pup(p);
    }
};

class Main : public CBase_Main
{
public:
    Main(CkArgMsg* msg)
    {
        double start = CkWallTimer();

        std::unique_ptr<vec_gen> gen = std::make_unique<vec_gen>();

        aum::vector A{1000000, std::move(gen)};
        aum::vector B{1000000, 2.2};
        aum::vector C{1000000, 3.3};
        aum::vector D{1000000, 4.4};

        // Force 2 temporaries
        aum::vector E = (A - D) - (B - C);

        // 1 temp to the left
        A = B - C - D;

        // 1 temp to the right
        B = C - (A - D);

        aum::wait_and_exit(B, start);
    }
};

#include "subtraction.def.h"

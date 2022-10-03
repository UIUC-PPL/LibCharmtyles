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

#include <charmtyles/charmtyles.hpp>

#include "base.decl.h"

class Main : public CBase_Main
{
public:
    Main(CkArgMsg* msg)
    {
        int num_pes = 6;
        if (msg->argc > 1)
            num_pes = atoi(msg->argv[1]);

        ct::init();
        thisProxy.benchmark();
    }

    void benchmark()
    {
        constexpr std::size_t vec_size_1 = 1 << 20;
        constexpr std::size_t vec_size_2 = 1 << 26;
        constexpr std::size_t vec_size_3 = 1 << 22;

        double start = CkWallTimer();

        ct::vector vec1{vec_size_1};
        ct::vector vec2{vec_size_1, 1.5};
        ct::vector vec3{vec_size_1, .5};

        ct::vector vec4 = vec1 + vec2 - vec3;

        ct::vector vec11{vec_size_2};
        ct::vector vec12{vec_size_2, 1.5};
        ct::vector vec13{vec_size_2, .5};

        ct::vector vec14 = vec11 + vec12 - vec13;

        ct::vector vec111{vec_size_3};
        ct::vector vec112{vec_size_3, 1.5};
        ct::vector vec113{vec_size_3, .5};

        ct::vector vec114 = vec111 + vec112 - vec113;

        // ct::vec_impl::vec_instr_queue_t& queue =
        //     CT_ACCESS_SINGLETON(ct::vec_impl::vec_instr_queue);
        // queue.print_instructions();

        ct::sync();

        double end = CkWallTimer();

        ckout << "Execution Time (Phase 1): " << end - start << endl;

        start = CkWallTimer();
        vec4 = vec1 - vec3 + vec4;
        vec14 = vec11 - vec13 + vec14;
        vec114 = vec111 - vec113 + vec114;

        ct::sync();
        end = CkWallTimer();

        ckout << "Execution Time (Phase 2): " << end - start << endl;

        // copy operator
        vec4 = vec1;
        // copy constructor
        ct::vector vec5 = vec4;

        ct::sync();

        ct::vector x{1 << 21, 1.0};
        ct::vector y{1 << 21, 2.0};
        ct::scalar scal1 = ct::dot(x, y);
        double underlying_val = scal1.get();

        ckout << "Result of Vector dot product: " << underlying_val << endl;

        CkExit();
    }
};

#include "base.def.h"

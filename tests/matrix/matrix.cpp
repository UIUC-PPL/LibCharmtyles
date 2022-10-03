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
        constexpr std::size_t mat_row_1 = 1 << 11;
        constexpr std::size_t mat_col_1 = 1 << 11;

        constexpr std::size_t mat_row_2 = 1 << 12;
        constexpr std::size_t mat_col_2 = 1 << 11;

        constexpr std::size_t mat_row_3 = 1 << 11;
        constexpr std::size_t mat_col_3 = 1 << 12;

        double start = CkWallTimer();

        ct::matrix mat1{mat_row_1, mat_col_1};
        ct::matrix mat2{mat_row_1, mat_col_1, 1.5};
        ct::matrix mat3{mat_row_1, mat_col_1, .5};

        ct::matrix mat4 = mat1 + mat2 - mat3;

        ct::matrix mat11{mat_row_2, mat_col_2};
        ct::matrix mat12{mat_row_2, mat_col_2, 1.5};
        ct::matrix mat13{mat_row_2, mat_col_2, .5};

        ct::matrix mat14 = mat11 + mat12 - mat13;

        ct::matrix mat111{mat_row_3, mat_col_3};
        ct::matrix mat112{mat_row_3, mat_col_3, 1.5};
        ct::matrix mat113{mat_row_3, mat_col_3, .5};

        ct::matrix mat114 = mat111 + mat112 - mat113;
        // ct::mat_impl::mat_instr_queue_t& queue =
        //     CT_ACCESS_SINGLETON(ct::mat_impl::mat_instr_queue);
        // queue.print_instructions();

        ct::sync();

        double end = CkWallTimer();

        ckout << "Execution Time (Phase 1): " << end - start << endl;

        start = CkWallTimer();
        mat4 = mat1 - mat3 + mat4;
        mat14 = mat11 - mat13 + mat14;
        mat114 = mat111 - mat113 + mat114;

        ct::sync();
        end = CkWallTimer();

        ckout << "Execution Time (Phase 2): " << end - start << endl;

        // copy operator
        mat4 = mat1;
        // copy constructor
        ct::matrix mat5 = mat4;

        ct::sync();

        // ct::matrix x{1 << 21, 1.0};
        // ct::matrix y{1 << 21, 2.0};
        // ct::scalar scal1 = ct::dot(x, y);
        // double underlying_val = scal1.get();

        // ckout << "Result of matrix dot product: " << underlying_val << endl;

        CkExit();
    }
};

#include "base.def.h"

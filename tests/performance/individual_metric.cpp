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

#include "individual_metric.decl.h"

class Main : public CBase_Main
{
    int vector_size;
    int matrix_r;
    int matrix_c;
    // int n;

    void print_info()
    {
        ckout << "Running Each Implemented Operation in LibAum..." << endl;
        ckout << "> Vector Size: " << vector_size << endl;
        ckout << "> Matrix Row Size: " << matrix_r
              << ", Matrix Col Size: " << matrix_c << endl;
        // ckout << "> Each execution times will be averaged over " << n
        //       << " executions" << endl;
    }

public:
    Main(CkArgMsg* msg)
    {
        vector_size = 1000000;
        matrix_r = 10000;
        matrix_c = 10000;
        // n = 3;

        for (int i = 1; i != msg->argc; i += 2)
        {
            const char* arg = msg->argv[i];

            if (std::strcmp(arg, "--vector-size") == 0)
            {
                CkAssert(msg->argv[i + 1]);
                vector_size = std::atoi(msg->argv[i + 1]);
            }
            else if (std::strcmp(arg, "--matrix-size") == 0)
            {
                CkAssert(msg->argv[i + 1]);
                matrix_r = std::atoi(msg->argv[i + 1]);
                matrix_c = std::atoi(msg->argv[i + 1]);
            }
            else if (std::strcmp(arg, "--matrix-row-size") == 0)
            {
                CkAssert(msg->argv[i + 1]);
                matrix_r = std::atoi(msg->argv[i + 1]);
            }
            else if (std::strcmp(arg, "--matrix-col-size") == 0)
            {
                CkAssert(msg->argv[i + 1]);
                matrix_c = std::atoi(msg->argv[i + 1]);
            }
            // else if (std::strcmp(arg, "--num-repeats") == 0)
            // {
            //     CkAssert(msg->argv[i + 1]);
            //     n = std::atoi(msg->argv[i + 1]);
            // }
            else
            {
                CkAbort("Incompatible argument. Exiting...");
            }
        }

        print_info();

        thisProxy.benchmark();
    }

    void benchmark()
    {
        vector_add();
        vector_sub();
        vector_dot();

        matrix_add();
        matrix_sub();
        mat_vec_dot();
        vec_mat_dot();

        CkExit();
    }

private:
    void vector_add()
    {
        ckout << "-- Benchmarking Vector Addition" << endl;

        double start = CkWallTimer();

        aum::vector v1{vector_size, 1.};
        aum::vector v2{vector_size, 1.};

        aum::vector v3 = v1 + v2;

        double val = aum::reduce_add(v3).get();

        double end = CkWallTimer();
        CkAssert(val == (vector_size * 2));

        ckout << "-- Execution Time: " << end - start << endl;
    }

    void vector_sub()
    {
        ckout << "-- Benchmarking Vector Subtraction" << endl;

        double start = CkWallTimer();

        aum::vector v1{vector_size, 2.};
        aum::vector v2{vector_size, 1.};

        aum::vector v3 = v1 - v2;

        double val = aum::reduce_add(v3).get();

        double end = CkWallTimer();
        CkAssert(val == vector_size);

        ckout << "-- Execution Time: " << end - start << endl;
    }

    void vector_dot()
    {
        ckout << "-- Benchmarking Vector Dot" << endl;

        double start = CkWallTimer();

        aum::vector v1{vector_size, 1.};
        aum::vector v2{vector_size, 1.};

        aum::scalar res = aum::dot(v1, v2);

        double val = res.get();

        double end = CkWallTimer();
        CkAssert(val == vector_size);

        ckout << "-- Execution Time: " << end - start << endl;
    }

    void matrix_add()
    {
        ckout << "-- Benchmarking Matrix Addition" << endl;

        double start = CkWallTimer();

        aum::matrix m1{matrix_r, matrix_c, 1.};
        aum::matrix m2{matrix_r, matrix_c, 1.};

        aum::matrix m3 = m1 + m2;

        double val = aum::reduce_add(m3).get();

        double end = CkWallTimer();
        CkAssert(val == (matrix_r * matrix_c * 2));

        ckout << "-- Execution Time: " << end - start << endl;
    }

    void matrix_sub()
    {
        ckout << "-- Benchmarking Matrix Subtraction" << endl;

        double start = CkWallTimer();

        aum::matrix m1{matrix_r, matrix_c, 2.};
        aum::matrix m2{matrix_r, matrix_c, 1.};

        aum::matrix m3 = m1 - m2;

        double val = aum::reduce_add(m3).get();

        double end = CkWallTimer();
        CkAssert(val == (matrix_r * matrix_c));

        ckout << "-- Execution Time: " << end - start << endl;
    }

    void mat_vec_dot()
    {
        ckout << "-- Benchmarking Matrix-Vector product" << endl;

        double start = CkWallTimer();

        aum::matrix m1{matrix_r, matrix_c, 1.};
        aum::vector v1{matrix_c, 1.};

        aum::vector res = aum::dot(m1, v1);

        double val = aum::reduce_add(res).get();

        double end = CkWallTimer();
        CkAssert(val == (matrix_r * matrix_c));

        ckout << "-- Execution Time: " << end - start << endl;
    }

    void vec_mat_dot()
    {
        ckout << "-- Benchmarking Vector-Matrix Product" << endl;

        double start = CkWallTimer();

        aum::matrix m1{matrix_r, matrix_c, 1.};
        aum::vector v1{matrix_c, 1.};

        aum::vector res = aum::dot(v1, m1);

        double val = aum::reduce_add(res).get();

        double end = CkWallTimer();
        CkAssert(val == (matrix_r * matrix_c));

        ckout << "-- Execution Time: " << end - start << endl;
    }
};

#include "individual_metric.def.h"

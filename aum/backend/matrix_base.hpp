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

#pragma once

#include "Matrix.decl.h"

#include <stdlib.h>
#include <vector>

#include <aum/util/sizes.hpp>

class Matrix : public CBase_Matrix
{
private:
    int dimy;
    int dimx;
    int num_chares_x;
    int num_chares_y;
    std::vector<double> mat;

    int READ_TAG;
    int WRITE_TAG;

public:
    Matrix_SDAG_CODE;

    Matrix(int dimx_, int dimy_, int numx, int numy)
      : num_chares_x(numx)
      , num_chares_y(numy)
      , mat()
      , READ_TAG(0)
      , WRITE_TAG(0)
    {
        dimx = aum::sizes::block_size::value_c;
        dimy = aum::sizes::block_size::value_r;

        if (dimx_ % aum::sizes::block_size::value_c != 0 &&
            thisIndex.x == num_chares_x - 1)
            dimx = dimx_ % aum::sizes::block_size::value_c;

        if (dimy_ % aum::sizes::block_size::value_r != 0 &&
            thisIndex.y == num_chares_y - 1)
            dimy = dimy_ % aum::sizes::block_size::value_r;

        mat = std::vector<double>(dimx * dimy);

        thisProxy(thisIndex.x, thisIndex.y).initialize_operation();
    }

    Matrix(int dimx_, int dimy_, double value, int numx, int numy)
      : dimy(dimy_)
      , dimx(dimx_)
      , num_chares_x(numx)
      , num_chares_y(numy)
      , mat()
      , READ_TAG(0)
      , WRITE_TAG(0)
    {
        dimx = aum::sizes::block_size::value_c;
        dimy = aum::sizes::block_size::value_r;

        if (dimx_ % aum::sizes::block_size::value_c != 0 &&
            thisIndex.x == num_chares_x - 1)
            dimx = dimx_ % aum::sizes::block_size::value_c;

        if (dimy_ % aum::sizes::block_size::value_r != 0 &&
            thisIndex.y == num_chares_y - 1)
            dimy = dimy_ % aum::sizes::block_size::value_r;

        mat = std::vector<double>(dimx * dimy, value);

        thisProxy(thisIndex.x, thisIndex.y).initialize_operation();
    }
};

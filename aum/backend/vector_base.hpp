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

#include "Vector.decl.h"

#include <vector>

class Vector : public CBase_Vector
{
private:
    int size;
    int num_chares;
    std::vector<double> vec;

    int READ_TAG;
    int WRITE_TAG;

public:
    Vector_SDAG_CODE;

    Vector(int size_, int num_chares_)
      : size(size_)
      , num_chares(num_chares_)
      , vec()
      , READ_TAG(0)
      , WRITE_TAG(0)
    {
        size = aum::sizes::array_size::value;

        if (size_ % aum::sizes::array_size::value != 0 &&
            thisIndex == num_chares - 1)
            size = size_ % aum::sizes::array_size::value;

        vec = std::vector<double>(size);

        thisProxy[thisIndex].initialize_operation();
    }

    Vector(int size_, double value, int num_chares_)
      : size(size_)
      , num_chares(num_chares_)
      , vec()
      , READ_TAG(0)
      , WRITE_TAG(0)
    {
        size = aum::sizes::array_size::value;

        if (size_ % aum::sizes::array_size::value != 0 &&
            thisIndex == num_chares - 1)
            size = size_ % aum::sizes::array_size::value;

        vec = std::vector<double>(size, value);

        thisProxy[thisIndex].initialize_operation();
    }
};

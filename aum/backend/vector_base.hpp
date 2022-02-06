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

struct part_vector_msg
  : public CkMcastBaseMsg
  , public CMessage_part_vector_msg
{
    CProxySection_Matrix sec;
    int size;
    double* arr;

    part_vector_msg(CProxySection_Matrix sec_, int size_, double* arr_)
      : sec(sec_)
      , size(size_)
      , arr(arr_)
    {
    }

    void pup(PUP::er& p)
    {
        CMessage_part_vector_msg::pup(p);
        p | sec;
        p | size;
        if (p.isUnpacking())
            arr = new double[size];
        p(arr, size);
    }
};

class Vector : public CBase_Vector
{
private:
    int size;
    int num_chares;
    std::vector<double> vec;

    int READ_TAG;
    int WRITE_TAG;

    int reduction_counter;

public:
    Vector_SDAG_CODE;

    Vector(int size_, int num_chares_)
      : size(size_)
      , num_chares(num_chares_)
      , vec()
      , READ_TAG(0)
      , WRITE_TAG(0)
      , reduction_counter(0)
    {
        size = aum::sizes::array_size::value;

        if (size_ % aum::sizes::array_size::value != 0 &&
            thisIndex == num_chares - 1)
            size = size_ % aum::sizes::array_size::value;

        vec = std::vector<double>(size);

        thisProxy[thisIndex].initialize_operation();
    }

    Vector(int size_, int num_chares_, aum::random)
      : size(size_)
      , num_chares(num_chares_)
      , vec()
      , READ_TAG(0)
      , WRITE_TAG(0)
      , reduction_counter(0)
    {
        size = aum::sizes::array_size::value;

        if (size_ % aum::sizes::array_size::value != 0 &&
            thisIndex == num_chares - 1)
            size = size_ % aum::sizes::array_size::value;

        vec.reserve(size);

        std::random_device rd;
        std::default_random_engine eng(rd());
        std::uniform_real_distribution<double> distr(0., 10.);

        for (int i = 0; i != size; ++i)
            vec.emplace_back(distr(eng));

        thisProxy[thisIndex].initialize_operation();
    }

    Vector(int size_, double value, int num_chares_)
      : size(size_)
      , num_chares(num_chares_)
      , vec()
      , READ_TAG(0)
      , WRITE_TAG(0)
      , reduction_counter(0)
    {
        size = aum::sizes::array_size::value;

        if (size_ % aum::sizes::array_size::value != 0 &&
            thisIndex == num_chares - 1)
            size = size_ % aum::sizes::array_size::value;

        vec = std::vector<double>(size, value);

        thisProxy[thisIndex].initialize_operation();
    }
};

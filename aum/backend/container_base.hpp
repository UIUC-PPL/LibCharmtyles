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

#include "Container.decl.h"

#include <aum/util/view.hpp>

class matrix_container : public CBase_matrix_container
{
private:
    int rows_;
    int cols_;
    aum::view<double, aum::matrix> mat;

    int num_chares_;
    int red_count;

    int READY;

public:
    matrix_container_SDAG_CODE;

    matrix_container(int rows, int cols)
      : rows_(rows)
      , cols_(cols)
      , mat(rows_, cols_)
      , red_count(0)
      , READY(0)
    {
        int num_chares_x = cols_ / aum::sizes::block_size::value_c;
        int num_chares_y = rows_ / aum::sizes::block_size::value_r;

        if (cols_ % aum::sizes::block_size::value_c != 0)
            ++num_chares_x;
        if (rows_ % aum::sizes::block_size::value_r != 0)
            ++num_chares_y;

        num_chares_ = num_chares_x * num_chares_y;

        thisProxy.initialize_operation();
    }
};

class vector_container : public CBase_vector_container
{
private:
    int size_;
    aum::view<double, aum::vector> vec;

    int num_chares_;
    int red_count;

    int READY;

public:
    vector_container_SDAG_CODE;

    vector_container(int size)
      : size_(size)
      , vec(size)
      , red_count(0)
      , READY(0)
    {
        num_chares_ = size_ / aum::sizes::array_size::value;

        if (size_ % aum::sizes::array_size::value)
            ++num_chares_;

        thisProxy.initialize_operation();
    }
};
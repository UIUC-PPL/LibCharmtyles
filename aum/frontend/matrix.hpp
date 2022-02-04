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

#include <aum/backend/matrix_base.hpp>

#include <aum/util/sizes.hpp>

#include <cassert>

namespace aum {

    // User facing aum::matrix class
    class Matrix
    {
    public:
        explicit Matrix(int rows, int cols)
          : rows_(rows)
          , cols_(cols)
          , num_chares_x_(cols_ % aum::sizes::block_size::value_c)
          , num_chares_y_(rows_ % aum::sizes::block_size::value_r)
          , read_tag_(0)
          , write_tag_(0)
        {
            if (rows_ % aum::sizes::block_size::value_r != 0)
                ++num_chares_y_;

            if (cols_ % aum::sizes::block_size::value_c != 0)
                ++num_chares_x_;

            proxy_ = CProxy_Matrix::ckNew(cols_, rows_, num_chares_x_,
                num_chares_y_, num_chares_x_, num_chares_y_);
        }

        explicit Matrix(int rows, int cols, double value)
          : rows_(rows)
          , cols_(cols)
          , num_chares_x_(cols_ % aum::sizes::block_size::value_c)
          , num_chares_y_(rows_ % aum::sizes::block_size::value_r)
          , read_tag_(0)
          , write_tag_(0)
        {
            if (rows_ % aum::sizes::block_size::value_r != 0)
                ++num_chares_y_;

            if (cols_ % aum::sizes::block_size::value_c != 0)
                ++num_chares_x_;

            proxy_ = CProxy_Matrix::ckNew(cols_, rows_, value, num_chares_x_,
                num_chares_y_, num_chares_x_, num_chares_y_);
        }

    private:
        int rows_;
        int cols_;
        int num_chares_x_;
        int num_chares_y_;
        mutable CProxy_Matrix proxy_;

        // Book-keeping variables
        mutable int read_tag_;
        mutable int write_tag_;
    };
}    // namespace aum

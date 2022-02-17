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

#include <aum/backend/exitter.hpp>
#include <aum/backend/matrix_base.hpp>

#include <aum/util/sizes.hpp>

#include <cassert>

namespace aum {

    // User facing aum::matrix class
    class matrix
    {
    public:
        explicit matrix(int rows, int cols)
          : rows_(rows)
          , cols_(cols)
          , num_chares_x_(cols_ / aum::sizes::block_size::value_c)
          , num_chares_y_(rows_ / aum::sizes::block_size::value_r)
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

        explicit matrix(int rows, int cols, aum::random)
          : rows_(rows)
          , cols_(cols)
          , num_chares_x_(cols_ / aum::sizes::block_size::value_c)
          , num_chares_y_(rows_ / aum::sizes::block_size::value_r)
          , read_tag_(0)
          , write_tag_(0)
        {
            if (rows_ % aum::sizes::block_size::value_r != 0)
                ++num_chares_y_;

            if (cols_ % aum::sizes::block_size::value_c != 0)
                ++num_chares_x_;

            proxy_ = CProxy_Matrix::ckNew(cols_, rows_, num_chares_x_,
                num_chares_y_, aum::random{}, num_chares_x_, num_chares_y_);
        }

        explicit matrix(int rows, int cols, double value)
          : rows_(rows)
          , cols_(cols)
          , num_chares_x_(cols_ / aum::sizes::block_size::value_c)
          , num_chares_y_(rows_ / aum::sizes::block_size::value_r)
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

        matrix(matrix const& other)
        {
            rows_ = other.rows();
            cols_ = other.cols();
            num_chares_x_ = other.num_chares_x();
            num_chares_y_ = other.num_chares_y();
            proxy_ = other.proxy();

            read_tag_ = other.read_tag();
            write_tag_ = other.write_tag();
        }

        matrix(matrix&& other)
        {
            rows_ = other.rows();
            cols_ = other.cols();
            num_chares_x_ = other.num_chares_x();
            num_chares_y_ = other.num_chares_y();
            proxy_ = other.proxy();

            read_tag_ = other.read_tag();
            write_tag_ = other.write_tag();
        }

        matrix& operator=(matrix const& other)
        {
            if (this == &other)
                return *this;

            rows_ = other.rows();
            cols_ = other.cols();
            num_chares_x_ = other.num_chares_x();
            num_chares_y_ = other.num_chares_y();
            proxy_ = other.proxy();

            read_tag_ = other.read_tag();
            write_tag_ = other.write_tag();

            return *this;
        }

        matrix& operator=(matrix&& other)
        {
            if (this == &other)
                return *this;

            rows_ = other.rows();
            cols_ = other.cols();
            num_chares_x_ = other.num_chares_x();
            num_chares_y_ = other.num_chares_y();
            proxy_ = other.proxy();

            read_tag_ = other.read_tag();
            write_tag_ = other.write_tag();

            return *this;
        }

        template <typename Container>
        void send_to_1(int result_tag, Container&& result) const;

        template <typename Container>
        void send_to_2(int result_tag, Container&& result) const;

        int rows() const
        {
            return rows_;
        }

        int cols() const
        {
            return cols_;
        }

        int num_chares_x() const
        {
            return num_chares_x_;
        }

        int num_chares_y() const
        {
            return num_chares_y_;
        }

        int read_tag() const
        {
            return read_tag_;
        }

        int write_tag() const
        {
            return write_tag_;
        }

        CProxy_Matrix proxy() const
        {
            return proxy_;
        }

        void update_tags()
        {
            ++write_tag_;
            read_tag_ = write_tag_ + 1;
        }

        void exit() const
        {
            proxy_.exit(read_tag_);
        }

        void exit(double start) const
        {
            CProxy_Exitter proxy = CProxy_Exitter::ckNew();
            proxy_.exit(read_tag_, start, proxy);
        }

        void inc_reads() const
        {
            ++write_tag_;
        }

        void print(std::string const& s) const
        {
            proxy_.print_value(read_tag_, s);
            inc_reads();
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

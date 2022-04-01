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

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>

namespace aum {

    class vector;
    class matrix;

    template <typename T, typename CT = aum::vector>
    class view
    {
    public:
        using value_type = T;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using view_type = CT;

        ~view()
        {
            std::free(data_);
        }

        explicit view()
          : size_(0)
          , data_(nullptr)
        {
        }

        explicit view(size_type size, value_type* data)
          : size_(size)
          , data_(data)
        {
        }

        explicit view(size_type size)
          : size_(size)
          , data_(nullptr)
        {
            data_ = reinterpret_cast<value_type*>(
                std::malloc(size * sizeof(value_type)));
        }

        explicit view(size_type size, value_type value)
          : size_(size)
          , data_(nullptr)
        {
            data_ = reinterpret_cast<value_type*>(
                std::malloc(size * sizeof(value_type)));

            std::fill(data_, data_ + size_, value);
        }

        void reserve(size_type size)
        {
            size_ = size;
            data_ = reinterpret_cast<value_type*>(
                std::malloc(size * sizeof(value_type)));
        }

        void reserve(size_type size, double value)
        {
            size_ = size;
            data_ = reinterpret_cast<value_type*>(
                std::malloc(size * sizeof(value_type)));

            std::fill(data_, data_ + size_, value);
        }

        value_type* begin()
        {
            return data_;
        }

        value_type* end()
        {
            return (data_ + size_);
        }

        size_t size() const
        {
            return size_;
        }

        const value_type* data() const
        {
            return data_;
        }

        value_type* data()
        {
            return data_;
        }

        value_type& operator[](size_type i)
        {
            return data_[i];
        }

        const value_type& operator[](size_type i) const
        {
            return data_[i];
        }

        void pup(PUP::er& p)
        {
            p | size_;

            if (p.isUnpacking())
            {
                data_ = new value_type[size_];
            }

            for (int i = 0; i != size_; ++i)
                p | data_[i];
        }

    private:
        size_type size_;
        value_type* data_;
    };

    template <typename T>
    class view<T, aum::matrix>
    {
    public:
        using value_type = T;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using view_type = aum::matrix;

        ~view()
        {
            std::free(data_);
        }

        explicit view()
          : dimx_(0)
          , dimy_(0)
          , size_(0)
          , data_(nullptr)
        {
        }

        explicit view(size_type dimx, size_type dimy, value_type* data)
          : dimx_(dimx)
          , dimy_(dimy)
          , size_(dimx * dimy)
          , data_(data)
        {
        }

        explicit view(size_type dimx, size_type dimy)
          : dimx_(dimx)
          , dimy_(dimy)
          , size_(dimx * dimy)
          , data_(nullptr)
        {
            data_ = reinterpret_cast<value_type*>(
                std::malloc(size_ * sizeof(value_type)));
        }

        explicit view(size_type dimx, size_type dimy, value_type value)
          : dimx_(dimx)
          , dimy_(dimy)
          , size_(dimx * dimy)
          , data_(nullptr)
        {
            data_ = reinterpret_cast<value_type*>(
                std::malloc(size_ * sizeof(value_type)));

            std::fill(data_, data_ + size_, value);
        }

        void reserve(size_type dimx, size_type dimy)
        {
            dimx_ = dimx;
            dimy_ = dimy;
            data_ = reinterpret_cast<value_type*>(
                std::malloc(size_ * sizeof(value_type)));
        }

        void reserve(size_type dimx, size_type dimy, double value)
        {
            dimx_ = dimx;
            dimy_ = dimy;
            data_ = reinterpret_cast<value_type*>(
                std::malloc(size_ * sizeof(value_type)));

            std::fill(data_, data_ + size_, value);
        }

        value_type* begin()
        {
            return data_;
        }

        value_type* end()
        {
            return (data_ + size_);
        }

        size_t size() const
        {
            return size_;
        }

        size_t dimx() const
        {
            return dimx_;
        }

        size_t dimy() const
        {
            return dimy_;
        }

        const value_type* data() const
        {
            return data_;
        }

        value_type* data()
        {
            return data_;
        }

        value_type& operator[](size_type i)
        {
            return data_[i];
        }

        const value_type& operator[](size_type i) const
        {
            return data_[i];
        }

        value_type& operator()(size_type x, size_type y)
        {
            return data_[y * dimx_ + x];
        }

        const value_type& operator()(size_type x, size_type y) const
        {
            return data_[y * dimx_ + x];
        }

        void pup(PUP::er& p)
        {
            p | dimx_;
            p | dimy_;
            p | size_;

            if (p.isUnpacking())
            {
                data_ = new value_type[size_];
            }

            for (int i = 0; i != size_; ++i)
                p | data_[i];
        }

    private:
        size_type dimx_;
        size_type dimy_;
        size_type size_;
        value_type* data_;
    };

}    // namespace aum

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
#include <aum/backend/vector_base.hpp>

#include <aum/util/sizes.hpp>

#include <cassert>

namespace aum {

    // User facing aum::vector class
    class vector
    {
    public:
        explicit vector(int size)
          : size_(size)
          , num_chares_(size_ / aum::sizes::array_size::value)
          , read_tag_(0)
          , write_tag_(0)
        {
            if (size_ % aum::sizes::array_size::value)
                ++num_chares_;

            proxy_ = CProxy_Vector::ckNew(size_, num_chares_, num_chares_);
        }

        explicit vector(int size, aum::random)
          : size_(size)
          , num_chares_(size_ / aum::sizes::array_size::value)
          , read_tag_(0)
          , write_tag_(0)
        {
            if (size_ % aum::sizes::array_size::value)
                ++num_chares_;

            proxy_ = CProxy_Vector::ckNew(
                size_, num_chares_, aum::random{}, num_chares_);
        }

        explicit vector(int size, double value)
          : size_(size)
          , num_chares_(size_ / aum::sizes::array_size::value)
          , read_tag_(0)
          , write_tag_(0)
        {
            if (size_ % aum::sizes::array_size::value)
                ++num_chares_;

            proxy_ =
                CProxy_Vector::ckNew(size_, value, num_chares_, num_chares_);
        }

        explicit vector(int size, std::unique_ptr<aum::generator>&& gen)
          : size_(size)
          , num_chares_(size_ / aum::sizes::array_size::value)
          , read_tag_(0)
          , write_tag_(0)
        {
            if (size_ % aum::sizes::array_size::value)
                ++num_chares_;

            proxy_ = CProxy_Vector::ckNew(
                size_, std::move(gen), num_chares_, num_chares_);
        }

        explicit vector(int size, double* data)
          : size_(size)
          , num_chares_(size_ / aum::sizes::array_size::value)
          , read_tag_(0)
          , write_tag_(0)
        {
            int tile_size = aum::sizes::array_size::value;
            if (size_ % aum::sizes::array_size::value)
                ++num_chares_;

            proxy_ = CProxy_Vector::ckNew(
                size_, num_chares_, true, num_chares_);

            for (int i = 0; i < num_chares_; i++)
            {
                if (size_ % aum::sizes::array_size::value != 0 &&
                    i == num_chares_ - 1)
                    tile_size = size_ % aum::sizes::array_size::value;
                proxy_[i].initialize_data(tile_size, 
                        data + i * aum::sizes::array_size::value);
            }
        }

        vector(vector const& other)
        {
            size_ = other.size();
            num_chares_ = other.num_chares();
            proxy_ = other.proxy();

            read_tag_ = other.read_tag();
            write_tag_ = other.write_tag();
        }

        vector(vector&& other)
        {
            size_ = other.size();
            num_chares_ = other.num_chares();
            proxy_ = other.proxy();

            read_tag_ = other.read_tag();
            write_tag_ = other.write_tag();
        }

        vector& operator=(vector const& other)
        {
            if (this == &other)
                return *this;

            size_ = other.size();
            num_chares_ = other.num_chares();
            proxy_ = other.proxy();

            read_tag_ = other.read_tag();
            write_tag_ = other.write_tag();

            return *this;
        }

        vector& operator=(vector&& other)
        {
            if (this == &other)
                return *this;

            size_ = other.size();
            num_chares_ = other.num_chares();
            proxy_ = other.proxy();

            read_tag_ = other.read_tag();
            write_tag_ = other.write_tag();

            return *this;
        }

        vector& add_inplace(vector const& other)
        {
            assert((size_ == other.size()) &&
                "Vectors provided with incompatible sizes");

            int w_tag = write_tag();
            other.send_to_1(w_tag, *this);
            proxy().plus_add(w_tag);
            update_tags();

            return *this;
        }

        vector& sub_inplace_1(vector const& other)
        {
            assert((size_ == other.size()) &&
                "Vectors provided with incompatible sizes");

            int w_tag = write_tag();
            other.send_to_1(w_tag, *this);
            proxy().minus_subtract(w_tag, false);
            update_tags();

            return *this;
        }

        vector& sub_inplace_2(vector const& other)
        {
            assert((size_ == other.size()) &&
                "Vectors provided with incompatible sizes");

            int w_tag = write_tag();
            other.send_to_1(w_tag, *this);
            proxy().minus_subtract(w_tag, true);
            update_tags();

            return *this;
        }

        vector copy() const
        {
            vector dest{size_};

            int w_tag = dest.write_tag();
            this->send_to_1(w_tag, dest);
            dest.proxy().copy_value(w_tag);
            dest.update_tags();

            return dest;
        }

        void copy(vector& dest) const
        {
            int w_tag = dest.write_tag();
            this->send_to_1(w_tag, dest);
            dest.proxy().copy_value(w_tag);
            dest.update_tags();
        }

        CProxy_Vector proxy() const
        {
            return proxy_;
        }

        int write_tag() const
        {
            return write_tag_;
        }

        int read_tag() const
        {
            return read_tag_;
        }

        int size() const
        {
            return size_;
        }

        int num_chares() const
        {
            return num_chares_;
        }

        void update_tags()
        {
            ++write_tag_;
            read_tag_ = write_tag_ + 1;
        }

        template <typename Container>
        void send_to_1(int result_tag, Container&& result) const;

        template <typename Container>
        void send_to_2(int result_tag, Container&& result) const;

        template <typename Container>
        void send_to_matrix_rows(int result_tag, Container&& result) const;

        template <typename Container>
        void send_to_matrix_rows_2(int result_tag, Container&& result) const;

        template <typename Container>
        void send_to_matrix_cols(int result_tag, Container&& result) const;

        void inc_reads() const
        {
            ++write_tag_;
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

        void print(std::string const& s) const
        {
            proxy_.print_value(read_tag_, s);
            inc_reads();
        }

    private:
        int size_;
        int num_chares_;
        mutable CProxy_Vector proxy_;

        // Book-keeping variables
        mutable int read_tag_;
        mutable int write_tag_;
    };

}    // namespace aum

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

#include <aum/backend/scalar_base.hpp>

#include <aum/util/sizes.hpp>

#include <cassert>

namespace aum {

    // User facing aum::scalar class
    class scalar
    {
    public:
        explicit scalar()
          : read_tag_(0)
          , write_tag_(0)
        {
            proxy_ = CProxy_Scalar::ckNew();
        }

        explicit scalar(double value)
          : read_tag_(0)
          , write_tag_(0)
        {
            proxy_ = CProxy_Scalar::ckNew(value);
        }

        scalar(scalar const& other)
        {
            proxy_ = other.proxy();

            read_tag_ = other.read_tag();
            write_tag_ = other.write_tag();
        }

        scalar(scalar&& other)
        {
            proxy_ = other.proxy();

            read_tag_ = other.read_tag();
            write_tag_ = other.write_tag();
        }

        scalar& operator=(scalar const& other)
        {
            if (this == &other)
                return *this;

            proxy_ = other.proxy();

            read_tag_ = other.read_tag();
            write_tag_ = other.write_tag();

            return *this;
        }

        scalar& operator=(scalar&& other)
        {
            if (this == &other)
                return *this;

            proxy_ = other.proxy();

            read_tag_ = other.read_tag();
            write_tag_ = other.write_tag();

            return *this;
        }

        scalar& add_inplace(scalar const& other)
        {
            int w_tag = write_tag();
            other.send_to_1(w_tag, *this);
            proxy().plus_add(w_tag);
            update_tags();

            return *this;
        }

        scalar& sub_inplace_1(scalar const& other)
        {
            int w_tag = write_tag();
            other.send_to_1(w_tag, *this);
            proxy().minus_subtract(w_tag, false);
            update_tags();

            return *this;
        }

        scalar& sub_inplace_2(scalar const& other)
        {
            int w_tag = write_tag();
            other.send_to_1(w_tag, *this);
            proxy().minus_subtract(w_tag, true);
            update_tags();

            return *this;
        }

        scalar copy() const
        {
            scalar dest{};

            int w_tag = dest.write_tag();
            this->send_to_1(w_tag, dest);
            dest.proxy().copy_value(w_tag);
            dest.update_tags();

            return dest;
        }

        void copy(scalar& dest) const
        {
            int w_tag = dest.write_tag();
            this->send_to_1(w_tag, dest);
            dest.proxy().copy_value(w_tag);
            dest.update_tags();
        }

        void print_value() const
        {
            ++write_tag_;

            proxy_.print_value(read_tag_);
        }

        double get() const
        {
            ck::future<double> f;

            proxy_.get_value(read_tag_, f);
            ++write_tag_;

            return f.get();
        }

        void print_value(std::string const& s) const
        {
            ++write_tag_;

            proxy_.print_value_string(read_tag_, s);
        }

        int write_tag() const
        {
            return write_tag_;
        }

        int read_tag() const
        {
            return read_tag_;
        }

        CProxy_Scalar proxy() const
        {
            return proxy_;
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
        void send_to_vector(int result_tag, Container&& result) const;

        template <typename Container>
        void send_to_matrix(int result_tag, Container&& result) const;

        void exit() const
        {
            proxy_.exit(read_tag_);
        }

        void exit(double start) const
        {
            proxy_.exit(read_tag_, start);
        }

    private:
        mutable CProxy_Scalar proxy_;

        // Book-Keeping variables
        mutable int read_tag_;
        mutable int write_tag_;
    };

}    // namespace aum

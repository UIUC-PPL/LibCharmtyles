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

        void print_value() const
        {
            ++write_tag_;

            proxy_.print_value(read_tag_);
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

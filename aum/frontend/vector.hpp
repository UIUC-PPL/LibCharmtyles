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

#include <aum/backend/vector_base.hpp>

#include <aum/util/sizes.hpp>

#include <cassert>

namespace aum {

    class vector;

    namespace detail {

        template <typename Callable>
        struct vector_op
        {
            aum::vector const& v1;
            aum::vector const& v2;

            // Callable to handle the operation
            Callable&& c;

            explicit vector_op(
                aum::vector const& v1_, aum::vector const& v2_, Callable&& c_)
              : v1(v1_)
              , v2(v2_)
              , c(std::move(c_))
            {
            }
        };

    }    // namespace detail

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

            proxy_ = CProxy_Vector::ckNew(
                aum::sizes::array_size::value, num_chares_);
        }

        explicit vector(int size, double value)
          : size_(size)
          , num_chares_(size_ / aum::sizes::array_size::value)
          , read_tag_(0)
          , write_tag_(0)
        {
            if (size_ % aum::sizes::array_size::value)
                ++num_chares_;

            proxy_ = CProxy_Vector::ckNew(
                aum::sizes::array_size::value, value, num_chares_);
        }

        template <typename Callable>
        vector(detail::vector_op<Callable> const& vop)
        {
            // Construct the proxy
            size_ = vop.v1.size();
            num_chares_ = vop.v1.num_chares();
            read_tag_ = 0;
            write_tag_ = 0;

            proxy_ = CProxy_Vector::ckNew(
                aum::sizes::array_size::value, num_chares_);

            vector const& v1 = vop.v1;
            vector const& v2 = vop.v2;

            vop.c(*this, v1, v2);
        }

        template <typename Callable>
        vector operator=(detail::vector_op<Callable> const& vop)
        {
            vector const& v1 = vop.v1;
            vector const& v2 = vop.v2;

            vop.c(*this, v1, v2);

            return *this;
        }

        CProxy_Vector proxy() const
        {
            return proxy_;
        }

        int write_tag() const
        {
            return write_tag_;
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
            read_tag_ = write_tag_ + 1;
        }

        void send_to_1(int result_tag, vector& result) const
        {
            ++write_tag_;

            proxy_.send_to_1(read_tag_, result_tag, result.proxy());
        }

        void send_to_2(int result_tag, vector& result) const
        {
            ++write_tag_;

            proxy_.send_to_2(read_tag_, result_tag, result.proxy());
        }

        void exit() const
        {
            proxy_.exit(read_tag_);
        }

    private:
        int size_;
        int num_chares_;
        mutable CProxy_Vector proxy_;

        // Book-keeping variables
        mutable int read_tag_;
        mutable int write_tag_;
    };

    auto operator+(vector const& v1, vector const& v2)
    {
        assert((v1.size() == v2.size()) &&
            "Vectors provided with incompatible sizes");

        return detail::vector_op{
            v1, v2, [](vector& result, vector const& v1, vector const& v2) {
                int w_tag = result.write_tag();
                v1.send_to_1(w_tag, result);
                v2.send_to_2(w_tag, result);
                result.proxy().add(w_tag);
                result.update_tags();
            }};
    }

}    // namespace aum

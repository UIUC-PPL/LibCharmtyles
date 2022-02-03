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

        vector(vector const& other)
        {
            size_ = other.size();
            num_chares_ = other.num_chares();
            proxy_ = other.proxy();

            read_tag_ = other.reads();
            write_tag_ = other.writes();
        }

        vector(vector&& other)
        {
            size_ = other.size();
            num_chares_ = other.num_chares();
            proxy_ = other.proxy();

            read_tag_ = other.reads();
            write_tag_ = other.writes();
        }

        vector& operator=(vector const& other)
        {
            if (this == &other)
                return *this;

            size_ = other.size();
            num_chares_ = other.num_chares();
            proxy_ = other.proxy();

            read_tag_ = other.reads();
            write_tag_ = other.writes();

            return *this;
        }

        vector& operator=(vector&& other)
        {
            if (this == &other)
                return *this;

            size_ = other.size();
            num_chares_ = other.num_chares();
            proxy_ = other.proxy();

            read_tag_ = other.reads();
            write_tag_ = other.writes();

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
            ++write_tag_;
            read_tag_ = write_tag_ + 1;
        }

        int reads() const
        {
            return read_tag_;
        }

        int writes() const
        {
            return write_tag_;
        }

        template <typename Vector>
        void send_to_1(int result_tag, Vector&& result) const
        {
            ++write_tag_;

            proxy_.send_to_1(read_tag_, result_tag, result.proxy());
        }

        template <typename Vector>
        void send_to_2(int result_tag, Vector&& result) const
        {
            ++write_tag_;

            proxy_.send_to_2(read_tag_, result_tag, result.proxy());
        }

        void exit() const
        {
            proxy_[0].exit(read_tag_);
        }

        void exit(double start) const
        {
            proxy_[0].exit(read_tag_, start);
        }

    private:
        int size_;
        int num_chares_;
        mutable CProxy_Vector proxy_;

        // Book-keeping variables
        mutable int read_tag_;
        mutable int write_tag_;
    };

    vector operator+(vector const& v1, vector const& v2)
    {
        assert((v1.size() == v2.size()) &&
            "Vectors provided with incompatible sizes");

        vector result{v1.size()};

        int w_tag = result.write_tag();
        v1.send_to_1(w_tag, result);
        v2.send_to_2(w_tag, result);
        result.proxy().add(w_tag);
        result.update_tags();

        return result;
    }

    vector operator+(vector&& v1, vector const& v2)
    {
        assert((v1.size() == v2.size()) &&
            "Vectors provided with incompatible sizes");

        int w_tag = v1.write_tag();
        v2.send_to_1(w_tag, v1);
        v1.proxy().plus_add(w_tag);
        v1.update_tags();

        return std::move(v1);
    }

    vector operator+(vector const& v1, vector&& v2)
    {
        assert((v1.size() == v2.size()) &&
            "Vectors provided with incompatible sizes");

        int w_tag = v2.write_tag();
        v1.send_to_1(w_tag, v2);
        v2.proxy().plus_add(w_tag);
        v2.update_tags();

        return std::move(v2);
    }

    vector operator+(vector&& v1, vector&& v2)
    {
        assert((v1.size() == v2.size()) &&
            "Vectors provided with incompatible sizes");

        int w_tag = v2.write_tag();
        v1.send_to_1(w_tag, v2);
        v2.proxy().plus_add(w_tag);
        v2.update_tags();

        return std::move(v2);
    }

    vector operator-(vector const& v1, vector const& v2)
    {
        assert((v1.size() == v2.size()) &&
            "Vectors provided with incompatible sizes");

        vector result{v1.size()};

        int w_tag = result.write_tag();
        v1.send_to_1(w_tag, result);
        v2.send_to_2(w_tag, result);
        result.proxy().subtract(w_tag);
        result.update_tags();

        return result;
    }

    vector operator-(vector&& v1, vector const& v2)
    {
        assert((v1.size() == v2.size()) &&
            "Vectors provided with incompatible sizes");

        int w_tag = v1.write_tag();
        v2.send_to_1(w_tag, v1);
        v1.proxy().minus_subtract(w_tag, false);
        v1.update_tags();

        return std::move(v1);
    }

    vector operator-(vector const& v1, vector&& v2)
    {
        assert((v1.size() == v2.size()) &&
            "Vectors provided with incompatible sizes");

        int w_tag = v2.write_tag();
        v1.send_to_1(w_tag, v2);
        v2.proxy().minus_subtract(w_tag, true);
        v2.update_tags();

        return std::move(v2);
    }

    vector operator-(vector&& v1, vector&& v2)
    {
        assert((v1.size() == v2.size()) &&
            "Vectors provided with incompatible sizes");

        int w_tag = v2.write_tag();
        v1.send_to_1(w_tag, v2);
        v2.proxy().minus_subtract(w_tag, true);
        v2.update_tags();

        return std::move(v2);
    }

}    // namespace aum

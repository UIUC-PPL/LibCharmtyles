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

class CProxy_Scalar;
class CProxy_Vector;
class CProxy_Matrix;
class CProxy_vector_container;
class CProxy_matrix_container;

#include <aum/backend/aum_base.hpp>
#include <aum/backend/container_base.hpp>
#include <aum/backend/exitter.hpp>

#include <aum/frontend/matrix.hpp>
#include <aum/frontend/scalar.hpp>
#include <aum/frontend/vector.hpp>

#include <aum/frontend/matrix_impl.hpp>
#include <aum/frontend/scalar_impl.hpp>
#include <aum/frontend/vector_impl.hpp>

#include <aum/algorithms/algorithms.hpp>

#include <aum/util/generator.hpp>
#include <aum/util/sizes.hpp>
#include <aum/util/view.hpp>

namespace aum {

    template <typename T>
    void wait_and_exit(T&& t)
    {
        t.exit();
    }

    template <typename T>
    void wait_and_exit(T&& t, double start)
    {
        t.exit(start);
    }

    namespace impl {
        template <typename Tuple, std::size_t I = 0>
        ck::future<bool> call_set(Tuple&& t)
        {
            ck::future<bool> f;
            decltype(std::get<I>(t)) container = std::get<I>(t);
            int w_tag = container.write_tag();
            container.proxy().wait_one(w_tag, f);
            container.update_tags();

            return f;
        }
    }    // namespace impl

    template <typename... Args>
    void wait_all(Args&&... args)
    {
        auto targs = std::make_tuple(args...);
        constexpr std::size_t tsize = std::tuple_size<decltype(targs)>{};

        std::array<ck::future<bool>, tsize> f_vec;
        for (int i = 0; i != tsize; ++i)
            f_vec[i] = impl::call_set(targs);

        for (int i = 0; i != tsize; ++i)
            f_vec[i].get();
    }

}    // namespace aum

#include <aum/backend/Container.def.h>
#include <aum/backend/CustomCallbacks.def.h>
#include <aum/backend/Exitter.def.h>
#include <aum/backend/Matrix.def.h>
#include <aum/backend/Scalar.def.h>
#include <aum/backend/Vector.def.h>
#include <aum/backend/libaum.def.h>

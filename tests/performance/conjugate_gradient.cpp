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

#include <aum/aum.hpp>

#include "conjugate_gradient.decl.h"

class Main : public CBase_Main
{
public:
    Main(CkArgMsg* msg)
    {
        thisProxy.benchmark();
    }

    void benchmark()
    {
        double start = CkWallTimer();

        // Initialized condition
        aum::matrix A{3000, 3000, 1.};
        aum::vector b{3000, aum::random{}};
        aum::vector x{3000, aum::random{}};

        aum::vector r = b - aum::dot(A, x);
        aum::vector p = aum::copy(r);
        aum::scalar rsold = aum::dot(r, r);

        for (int i = 0; i != 100; ++i)
        {
            aum::vector Ap = aum::dot(A, p);
            aum::scalar alpha = rsold / aum::dot(p, Ap);

            x = aum::blas::axpy(alpha, p, x);
            r = aum::blas::axpy(-1., alpha, Ap, r);

            aum::scalar rsnew = aum::dot(r, r);

            double rsnew_value = rsnew.get();
            if (std::sqrt(rsnew_value) < 1E-8)
            {
                ckout << "Converged in " << i << " iterations" << endl;
                break;
            }

            p = aum::blas::axpy(rsnew / rsold, p, r);
            rsold = rsnew;
        }

        aum::wait_and_exit(r, start);
    }
};

#include "conjugate_gradient.def.h"

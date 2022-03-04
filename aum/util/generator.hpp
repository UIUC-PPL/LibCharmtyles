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

#include "charm++.h"

namespace aum {

    class generator : public PUP::able
    {
    public:
        PUPable_decl(generator);

        generator() = default;

        generator(CkMigrateMessage* m)
          : PUP::able(m)
        {
        }

        virtual void pup(PUP::er& p)
        {
            PUP::able::pup(p);
        }

        // Default generate function for matrix
        virtual void generate(
            int dimX, int dimY, int matDimX, int matDimY, double* data)
        {
            return;
        }

        // Default generate function for vector
        virtual void generate(int index, int size, double* data)
        {
            return;
        }
    };

}    // namespace aum

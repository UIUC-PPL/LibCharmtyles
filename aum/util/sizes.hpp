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

#ifndef AUM_ARRAY_SIZE
#define AUM_ARRAY_SIZE 10000
#endif

#ifndef AUM_BLOCK_ROW_SIZE
#define AUM_BLOCK_ROW_SIZE 100
#endif

#ifndef AUM_BLOCK_COL_SIZE
#define AUM_BLOCK_COL_SIZE 100
#endif

namespace aum {
    namespace sizes {

        struct array_size
        {
            constexpr static int value = AUM_ARRAY_SIZE;
        };

        struct block_size
        {
            constexpr static int value_r = AUM_BLOCK_ROW_SIZE;
            constexpr static int value_c = AUM_BLOCK_COL_SIZE;
            constexpr static int value =
                AUM_BLOCK_COL_SIZE * AUM_BLOCK_ROW_SIZE;
        };

    }    // namespace sizes

    struct random
    {
        void pup(PUP::er&) {}
    };

}    // namespace aum

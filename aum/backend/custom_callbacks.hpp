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

#include "aum/backend/CustomCallbacks.decl.h"

/*global*/ CkReduction::reducerType MaxIndexType;

struct IndexPack
{
    int index;
    double value;

    IndexPack() = default;

    explicit IndexPack(int index_, double value_)
      : index(index_)
      , value(value_)
    {
    }

    void pup(PUP::er& p)
    {
        p | index;
        p | value;
    }
};

CkReductionMsg* MaxIndex(int nMsg, CkReductionMsg** msgs)
{
    // Sum starts off at zero
    IndexPack* p = (IndexPack*) msgs[0]->getData();
    int max_index = p->index;
    double max_val = p->value;

    for (int i = 1; i < nMsg; ++i)
    {
        // Sanity check:
        CkAssert(msgs[i]->getSize() == (sizeof(IndexPack)));
        // Extract this message's data
        IndexPack* pack = (IndexPack*) msgs[i]->getData();
        if (pack->value > max_val)
        {
            max_val = pack->value;
            max_index = pack->index;
        }
    }

    IndexPack ret{max_index, max_val};

    return CkReductionMsg::buildNew(sizeof(IndexPack), &ret);
}

/*initnode*/ void registerMaxIndex(void)
{
    MaxIndexType = CkReduction::addReducer(MaxIndex);
}

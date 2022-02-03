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

#include "Scalar.decl.h"

#include <stdlib.h>

class Scalar : public CBase_Scalar
{
private:
    double value;

    int READ_TAG;
    int WRITE_TAG;

public:
    Scalar_SDAG_CODE;

    Scalar(double value_)
      : value(value_)
      , READ_TAG(0)
      , WRITE_TAG(0)
    {
        thisProxy.initialize_operation();
    }

    Scalar()
      : value(0.0)
      , READ_TAG(0)
      , WRITE_TAG(0)
    {
        thisProxy.initialize_operation();
    }
};

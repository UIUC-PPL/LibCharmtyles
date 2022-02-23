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

#include "libaum.decl.h"

struct vector_msg : CMessage_vector_msg
{
    int size;
<<<<<<< HEAD
    int tag;
    CkCallback cb;
=======
    CkCallback cb;
    int tag;
>>>>>>> main
    double* local;
    double* arr;

    static void* pack(vector_msg* msg)
    {
        if (msg->local != nullptr)
        {
            msg->arr =
                (double*) ((char*) msg + ALIGN_DEFAULT(sizeof(vector_msg)));

            std::copy(msg->local, msg->local + msg->size, msg->arr);
            msg->local = nullptr;
        }

        // Set an offset to the array
        msg->arr = (double*) ((char*) msg->arr - (char*) msg);

        return msg;
    }

    static vector_msg* unpack(void* buf)
    {
        vector_msg* msg = (vector_msg*) buf;
        msg->arr = (double*) ((size_t) msg->arr + (char*) msg);
        return msg;
    }
};

vector_msg* make_vector_msg(int size_, double* arr_, int tag)
{
    auto* msg = new (&size_) vector_msg();
    msg->size = size_;
<<<<<<< HEAD
    msg->tag = tag;
    msg->cb = CkCallback(CkCallback::invalid);
=======
>>>>>>> main

    // Just copy the pointers for now. We will pack the remaining if we're
    // sending it off node.
    msg->local = arr_;
    msg->arr = arr_;

<<<<<<< HEAD
=======
    msg->tag = tag;

>>>>>>> main
    return msg;
}

struct matrix_msg : CMessage_matrix_msg
{
    int dimx;
    int dimy;
<<<<<<< HEAD
    int tag;
    CkCallback cb;
=======
    CkCallback cb;
    int tag;
>>>>>>> main
    double* local;
    double* mat;

    static void* pack(matrix_msg* msg)
    {
        if (msg->local != nullptr)
        {
            msg->mat =
                (double*) ((char*) msg + ALIGN_DEFAULT(sizeof(matrix_msg)));

            std::copy(
                msg->local, msg->local + (msg->dimx * msg->dimy), msg->mat);
            msg->local = nullptr;
        }

        // Set an offset to the array
        msg->mat = (double*) ((char*) msg->mat - (char*) msg);

        return msg;
    }

    static matrix_msg* unpack(void* buf)
    {
        matrix_msg* msg = (matrix_msg*) buf;
        msg->mat = (double*) ((size_t) msg->mat + (char*) msg);
        return msg;
    }
};

matrix_msg* make_matrix_msg(int dimx, int dimy, double* arr_, int tag)
{
    auto* msg = new (dimx * dimy) matrix_msg();
    msg->dimx = dimx;
    msg->dimy = dimy;

<<<<<<< HEAD
    msg->tag = tag;
    msg->cb = CkCallback(CkCallback::invalid);

=======
>>>>>>> main
    // Just copy the pointers for now. We will pack the remaining if we're
    // sending it off node.
    msg->local = arr_;
    msg->mat = arr_;
<<<<<<< HEAD
=======
    msg->tag = tag;
>>>>>>> main

    return msg;
}

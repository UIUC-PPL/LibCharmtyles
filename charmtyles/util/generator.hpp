#pragma once

#include "charm++.h"

namespace ct {

    class generator : public PUP::able
    {
    public:
        PUPable_decl(generator);

        generator() = default;
        virtual ~generator() = default;

        generator(CkMigrateMessage* m)
          : PUP::able(m)
        {
        }

        virtual void pup(PUP::er& p)
        {
            PUP::able::pup(p);
        }

        // Default generate function for matrix
        virtual double generate(int row_id, int col_id)
        {
            return 0.;
        }

        // Default generate function for vector
        virtual double generate(int dimX)
        {
            return 0.;
        }
    };

    class unary_operator : public PUP::able
    {
    public:
        PUPable_decl(unary_operator);

        unary_operator() = default;
        virtual ~unary_operator() = default;

        unary_operator(CkMigrateMessage* m)
          : PUP::able(m)
        {
        }

        // Default Operator overload for vectors
        virtual double operator()(std::size_t index, double value)
        {
            return -1.0;
        }

        // Default Operator overload for matrices
        virtual double operator()(
            std::size_t row_id, std::size_t col_id, double value)
        {
            return -1.0;
        }
    };

    class binary_operator : public PUP::able
    {
    public:
        PUPable_decl(binary_operator);

        binary_operator() = default;
        virtual ~binary_operator() = default;

        binary_operator(CkMigrateMessage* m)
          : PUP::able(m)
        {
        }

        virtual void pup(PUP::er& p)
        {
            PUP::able::pup(p);
        }

        virtual double operator()(
            std::size_t index, double left_val, double right_val)
        {
            return -1.0;
        }

        virtual double operator()(std::size_t row_id, std::size_t col_id,
            double left_val, double right_val)
        {
            return -1.0;
        }
    };

}    // namespace ct

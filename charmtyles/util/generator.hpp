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

}    // namespace ct

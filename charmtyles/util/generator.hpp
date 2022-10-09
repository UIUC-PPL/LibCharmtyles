#pragma once

#include "charm++.h"

#include <charmtyles/util/matrix_view.hpp>

namespace ct { namespace util {

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
        virtual void generate(std::size_t chare_idx_x, std::size_t chare_idx_y,
            ct::util::matrix_view& mat)
        {
            return;
        }

        // Default generate function for vector
        virtual void generate(std::size_t chare_idx, std::vector<double>& vec)
        {
            return;
        }
    };

}}    // namespace ct::util

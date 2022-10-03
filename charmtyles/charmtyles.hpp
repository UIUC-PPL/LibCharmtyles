#pragma once

#include <charmtyles/backend/charmtyles_base.hpp>

#include <charmtyles/util/AST.hpp>
#include <charmtyles/util/singleton.hpp>
#include <charmtyles/util/sizes.hpp>

#include <charmtyles/frontend/operations.hpp>
#include <charmtyles/frontend/vector.hpp>

namespace ct {

    void init()
    {
        scalar_impl_proxy = CProxy_scalar_impl::ckNew();

        std::size_t& vec_len = CT_ACCESS_SINGLETON(ct::util::array_block_len);
        vec_len = 1 << 20;

        ckout << "Vector Block Length Set to: " << vec_len << endl;

        std::size_t& row_len = CT_ACCESS_SINGLETON(ct::util::matrix_block_rows);
        row_len = 1 << 10;

        ckout << "Matrix Row Block Length Set to: " << row_len << endl;

        std::size_t& col_len = CT_ACCESS_SINGLETON(ct::util::matrix_block_cols);
        col_len = 1 << 10;

        ckout << "Matrix Col Block Length Set to: " << col_len << endl;
    }

    void sync()
    {
        ct::vec_impl::vec_instr_queue_t& queue =
            CT_ACCESS_SINGLETON(ct::vec_impl::vec_instr_queue);
        int dispatch_count = queue.dispatch_size();

        ck::future<bool> is_done;

        CProxy_set_future proxy =
            CProxy_set_future::ckNew(is_done, dispatch_count);
        queue.dispatch(is_done, proxy);

        is_done.get();
        is_done.release();
        return;
    }

}    // namespace ct

#include <charmtyles/backend/libcharmtyles.def.h>

#pragma once

#include <charmtyles/backend/charmtyles_base.hpp>

#include <charmtyles/util/AST.hpp>
#include <charmtyles/util/singleton.hpp>
#include <charmtyles/util/sizes.hpp>

#include <charmtyles/frontend/matrix.hpp>
#include <charmtyles/frontend/operations.hpp>
#include <charmtyles/frontend/scalar.hpp>
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
        ct::vec_impl::vec_instr_queue_t& vec_queue =
            CT_ACCESS_SINGLETON(ct::vec_impl::vec_instr_queue);
        int vec_dispatch_count = vec_queue.dispatch_size();

        ct::mat_impl::mat_instr_queue_t& mat_queue =
            CT_ACCESS_SINGLETON(ct::mat_impl::mat_instr_queue);
        int mat_dispatch_count = mat_queue.dispatch_size();

        ck::future<bool> vec_is_done;
        ck::future<bool> mat_is_done;

        CProxy_set_future vec_proxy =
            CProxy_set_future::ckNew(vec_is_done, vec_dispatch_count);
        CProxy_set_future mat_proxy =
            CProxy_set_future::ckNew(mat_is_done, mat_dispatch_count);
        vec_queue.dispatch(vec_is_done, vec_proxy);
        mat_queue.dispatch(mat_is_done, mat_proxy);

        vec_is_done.get();
        vec_is_done.release();

        mat_is_done.get();
        mat_is_done.release();

        return;
    }

}    // namespace ct

#include <charmtyles/backend/libcharmtyles.def.h>

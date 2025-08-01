#pragma once

#include <charmtyles/backend/charmtyles_base.hpp>

#include <charmtyles/util/AST.hpp>
#include <charmtyles/util/generator.hpp>
#include <charmtyles/util/matrix_view.hpp>
#include <charmtyles/util/singleton.hpp>
#include <charmtyles/util/sizes.hpp>

#include <charmtyles/frontend/matrix.hpp>
#include <charmtyles/frontend/operations.hpp>
#include <charmtyles/frontend/scalar.hpp>
#include <charmtyles/frontend/vector.hpp>
#include <charmtyles/frontend/basic_unary_operators.hpp>
#include <charmtyles/frontend/basic_binary_operators.hpp>

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

    void sync(ct::mat_impl::mat_shape_t const& matrix_shape)
    {
        ct::mat_impl::mat_instr_queue_t& mat_queue =
            CT_ACCESS_SINGLETON(ct::mat_impl::mat_instr_queue);

        ck::future<bool> mat_sync;
        CProxy_set_future mat_proxy = CProxy_set_future::ckNew(mat_sync, 1);

        mat_queue.sync(matrix_shape.shape_id, mat_proxy);

        mat_sync.get();
        mat_sync.release();

        return;
    }

    void sync(ct::vec_impl::vec_shape_t const& vector_shape)
    {
        ct::vec_impl::vec_instr_queue_t& vec_queue =
            CT_ACCESS_SINGLETON(ct::vec_impl::vec_instr_queue);

        ck::future<bool> vec_sync;
        CProxy_set_future vec_proxy = CProxy_set_future::ckNew(vec_sync, 1);

        vec_queue.sync(vector_shape.shape_id, vec_proxy);

        vec_sync.get();
        vec_sync.release();

        return;
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

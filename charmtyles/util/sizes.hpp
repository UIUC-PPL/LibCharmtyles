#pragma once

#include <cstdint>

#include <charmtyles/util/singleton.hpp>

namespace ct { namespace util {

    CT_GENERATE_SINGLETON(std::size_t, array_block_len);

    CT_GENERATE_SINGLETON(std::size_t, matrix_block_cols);
    CT_GENERATE_SINGLETON(std::size_t, matrix_block_rows);

}}    // namespace ct::util

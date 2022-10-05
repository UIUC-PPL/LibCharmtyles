#pragma once

#include <cstdint>
#include <vector>

#include "charm++.h"

namespace ct { namespace util {

    class matrix_view
    {
    public:
        matrix_view() = default;

        matrix_view(matrix_view const& other) = default;
        matrix_view(matrix_view&& other) = default;
        matrix_view& operator=(matrix_view const& other) = default;
        matrix_view& operator=(matrix_view&& other) = default;

        explicit matrix_view(std::size_t rows, std::size_t cols)
          : rows_(rows)
          , cols_(cols)
          , mat_(rows * cols)
        {
        }

        explicit matrix_view(std::size_t rows, std::size_t cols, double value)
          : rows_(rows)
          , cols_(cols)
          , mat_(rows * cols, value)
        {
        }

        double& operator()(std::size_t row, std::size_t col)
        {
            return mat_[row * cols_ + col];
        }

        const double& operator()(std::size_t row, std::size_t col) const
        {
            return mat_[row * cols_ + col];
        }

        std::size_t rows() const
        {
            return rows_;
        }

        std::size_t cols() const
        {
            return cols_;
        }

        double* data()
        {
            return mat_.data();
        }

        const double* data() const
        {
            return mat_.data();
        }

        void pup(PUP::er& p)
        {
            p | rows_;
            p | cols_;
            p | mat_;
        }

    private:
        std::size_t rows_;
        std::size_t cols_;
        std::vector<double> mat_;
    };
}}    // namespace ct::util

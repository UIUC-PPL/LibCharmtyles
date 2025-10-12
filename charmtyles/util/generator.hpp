#pragma once

#include "charm++.h"
#include <Kokkos_Core.hpp>
#include <vector>

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

    class from_vector_generator : public generator
    {
    public:
        from_vector_generator() = default;
        ~from_vector_generator() {}

        using ct::generator::generator;

        from_vector_generator(const double* data, uint64_t size)
          : data_(std::vector<double>(data, data + size))
        {
        }

        from_vector_generator(const std::vector<double>& data)
          : data_(data)
        {
        }

        // returns the element at dimX
        double generate(int dimX) final
        {
            return data_[dimX];
        }

        double generate(int row_id, int col_id) final
        {
            return 0.0;    // Not used
        }

        PUPable_decl(from_vector_generator);
        from_vector_generator(CkMigrateMessage* m)
          : ct::generator(m)
        {
        }

        void pup(PUP::er& p) final
        {
            ct::generator::pup(p);
            p | data_;
        }

    private:
        std::vector<double> data_;
    };

    class from_matrix_generator : public generator
    {
    public:
        from_matrix_generator() = default;
        ~from_matrix_generator() {}

        using ct::generator::generator;

        from_matrix_generator(const double* data, uint16_t rows, uint16_t cols)
        {
            data_.resize(rows);
            for (uint16_t i = 0; i < rows; ++i)
            {
                data_[i].resize(cols);
                std::copy(
                    data + i * cols, data + (i + 1) * cols, data_[i].begin());
            }
        }

        from_matrix_generator(const std::vector<std::vector<double>>& data)
          : data_(data)
        {
        }

        // returns the element at dimX
        double generate(int dimX) final
        {
            return 0.0;    // Not used
        }

        double generate(int row_id, int col_id) final
        {
            return data_[row_id][col_id];    // Not used
        }

        PUPable_decl(from_matrix_generator);
        from_matrix_generator(CkMigrateMessage* m)
          : ct::generator(m)
        {
        }

        void pup(PUP::er& p) final
        {
            ct::generator::pup(p);
            p | data_;
        }

    private:
        std::vector<std::vector<double>> data_;
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

        virtual void pup(PUP::er& p)
        {
            PUP::able::pup(p);
        }
        virtual std::string get_vec_signature(){
            return "";
        }

        virtual std::string get_mat_signature(){
            return "";
        }
        virtual std::vector<double> get_extra_params(){
            return {};
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
        virtual std::string get_vec_signature(){
            return "";
        }

        virtual std::string get_mat_signature(){
            return "";
        }
        virtual std::vector<double> get_extra_params(){
            return {};
        }
    };

    class custom_operator : public PUP::able
    {
    public:
        PUPable_decl(custom_operator);

        custom_operator() = default;
        virtual ~custom_operator() = default;

        custom_operator(CkMigrateMessage* m)
          : PUP::able(m)
        {
        }

        virtual void pup(PUP::er& p)
        {
            PUP::able::pup(p);
        }

        virtual void operator()(std::size_t length, std::vector<double>& lhs,
            std::vector<double>& rhs)
        {
            lhs = rhs;
        }

        virtual void operator()(std::size_t rows, std::size_t cols, std::vector<std::vector<double>>& lhs,  std::vector<std::vector<double>>& rhs)
        {
            // Default implementation: copy rhs to lhs
            lhs = rhs;
        }
    };

}    // namespace ct

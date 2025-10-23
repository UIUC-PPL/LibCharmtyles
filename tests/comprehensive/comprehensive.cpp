#include <charmtyles/charmtyles.hpp>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <numeric>
#include <string>
#include <vector>

#include "base.decl.h"

namespace {

constexpr double kTolerance = 1e-10;

void fail(const std::string& test_name, const std::string& reason)
{
    ckout << "[FAIL] " << test_name.c_str() << ": " << reason.c_str() << endl;
    CkAbort("Comprehensive test suite failure");
}

void assert_vector_equals(const std::vector<double>& actual,
    const std::vector<double>& expected, const std::string& test_name)
{
    if (actual.size() != expected.size())
    {
        fail(test_name,
            "Size mismatch. Expected " + std::to_string(expected.size())
                + " got " + std::to_string(actual.size()));
    }

    for (std::size_t i = 0; i < actual.size(); ++i)
    {
        if (std::abs(actual[i] - expected[i]) > kTolerance)
        {
            fail(test_name,
                "Value mismatch at index " + std::to_string(i) + ". Expected "
                    + std::to_string(expected[i]) + " got "
                    + std::to_string(actual[i]));
        }
    }

    ckout << "[PASS] " << test_name.c_str() << endl;
}

void assert_matrix_equals(const std::vector<std::vector<double>>& actual,
    const std::vector<std::vector<double>>& expected,
    const std::string& test_name)
{
    if (actual.size() != expected.size())
    {
        fail(test_name,
            "Row mismatch. Expected " + std::to_string(expected.size())
                + " got " + std::to_string(actual.size()));
    }

    for (std::size_t row = 0; row < actual.size(); ++row)
    {
        if (actual[row].size() != expected[row].size())
        {
            fail(test_name,
                "Column mismatch in row " + std::to_string(row) + ". Expected "
                    + std::to_string(expected[row].size()) + " got "
                    + std::to_string(actual[row].size()));
        }

        for (std::size_t col = 0; col < actual[row].size(); ++col)
        {
            if (std::abs(actual[row][col] - expected[row][col]) > kTolerance)
            {
                fail(test_name,
                    "Value mismatch at (" + std::to_string(row) + ", "
                        + std::to_string(col) + "). Expected "
                        + std::to_string(expected[row][col]) + " got "
                        + std::to_string(actual[row][col]));
            }
        }
    }

    ckout << "[PASS] " << test_name.c_str() << endl;
}

}    // namespace

class Main : public CBase_Main
{
public:
    explicit Main(CkArgMsg* msg)
    {
        int num_pes = 4;
        if (msg->argc > 1)
            num_pes = atoi(msg->argv[1]);

        ckout << "[INFO] Initializing Charmtyles comprehensive test suite on "
              << num_pes << " PEs" << endl;
        ct::init();
        thisProxy.run();
    }

    void run()
    {
        test_vector_addition();
        test_vector_division();
        test_matrix_addition();
        test_matrix_division();
        test_vector_vector_dot();
        test_matrix_vector_dot();

        ckout << "[SUCCESS] Comprehensive Charmtyles tests passed" << endl;
        CkExit();
    }

private:
    static std::vector<double> fill_vector(double value, std::size_t size)
    {
        return std::vector<double>(size, value);
    }

    static std::vector<std::vector<double>> fill_matrix(
        double value, std::size_t rows, std::size_t cols)
    {
        return std::vector<std::vector<double>>(
            rows, std::vector<double>(cols, value));
    }

    void test_vector_addition()
    {
        constexpr std::size_t size = 8;
        const std::string test_name = "Vector addition";

        ct::vector lhs{size, 1.25};
        ct::vector rhs{size, 2.75};
        ct::vector result = lhs + rhs;
        ct::sync();

        assert_vector_equals(
            result.get(), fill_vector(4.0, size), test_name);
    }

    void test_vector_division()
    {
        constexpr std::size_t size = 6;
        const std::string test_name = "Vector division";

        ct::vector lhs{size, 6.0};
        ct::vector rhs{size, -2.0};
        ct::vector result = lhs / rhs;
        ct::sync();

        assert_vector_equals(
            result.get(), fill_vector(-3.0, size), test_name);
    }

    void test_matrix_addition()
    {
        constexpr std::size_t rows = 4;
        constexpr std::size_t cols = 5;
        const std::string test_name = "Matrix addition";

        ct::matrix lhs{rows, cols, 1.0};
        ct::matrix rhs{rows, cols, 2.0};
        ct::matrix result = lhs + rhs;
        ct::sync();

        assert_matrix_equals(
            result.get(), fill_matrix(3.0, rows, cols), test_name);
    }

    void test_matrix_division()
    {
        const std::string test_name = "Matrix division";

        constexpr std::size_t rows = 3;
        constexpr std::size_t cols = 2;

        ct::matrix lhs{rows, cols, 12.0};
        ct::matrix rhs{rows, cols, -4.0};
        ct::matrix result = lhs / rhs;
        ct::sync();

        assert_matrix_equals(
            result.get(), fill_matrix(-3.0, rows, cols), test_name);
    }

    void test_vector_vector_dot()
    {
        const std::string test_name = "Vector dot product";

        const std::vector<double> lhs_vals{1.0, 2.0, 3.0, 4.0};
        const std::vector<double> rhs_vals{5.0, 6.0, 7.0, 8.0};

        ct::vector lhs = ct::from_vector(lhs_vals);
        ct::vector rhs = ct::from_vector(rhs_vals);
        ct::scalar result = ct::dot(lhs, rhs);
        ct::sync();

        const double expected =
            std::inner_product(lhs_vals.begin(), lhs_vals.end(),
                rhs_vals.begin(), 0.0);

        double actual = result.get();
        if (std::abs(actual - expected) > kTolerance)
        {
            fail(test_name,
                "Value mismatch. Expected " + std::to_string(expected)
                    + " got " + std::to_string(actual));
        }

        ckout << "[PASS] " << test_name.c_str() << endl;
    }

    void test_matrix_vector_dot()
    {
        const std::string test_name = "Matrix-vector dot product";

        const std::vector<std::vector<double>> mat_vals{
            {2.0, 0.0, 1.0},
            {-1.0, 3.0, 2.0},
            {4.0, -2.0, 0.5}};
        const std::vector<double> vec_vals{4.0, -2.0, 1.0};

        ct::matrix mat = ct::from_matrix(mat_vals);
        ct::vector vec = ct::from_vector(vec_vals);

        ct::vector result = ct::dot(mat, vec);
        ct::sync();

        std::vector<double> expected;
        expected.reserve(mat_vals.size());
        for (const auto& row : mat_vals)
        {
            expected.push_back(std::inner_product(
                row.begin(), row.end(), vec_vals.begin(), 0.0));
        }

        assert_vector_equals(result.get(), expected, test_name);
    }
};

#include "base.def.h"

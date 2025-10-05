#include <charmtyles/charmtyles.hpp>

#include "base.decl.h"

class Main : public CBase_Main
{
public:
    Main(CkArgMsg* msg)
    {
        int num_pes = 6;
        if (msg->argc > 1)
            num_pes = atoi(msg->argv[1]);

        ct::init();
        thisProxy.benchmark();
    }

    void benchmark()
    {
        constexpr std::size_t mat_row_1 = 1 << 11;
        constexpr std::size_t mat_col_1 = 1 << 11;

        constexpr std::size_t mat_row_2 = 1 << 12;
        constexpr std::size_t mat_col_2 = 1 << 11;

        constexpr std::size_t mat_row_3 = 1 << 11;
        constexpr std::size_t mat_col_3 = 1 << 12;

        auto verify_mat = [&](const std::vector<std::vector<double>>& mdata,
                            std::size_t rows, std::size_t cols,
                            double expected, const char* name) {
            const double eps = 1e-12; // tolerance for floating comparisons
            if (mdata.size() != rows) {
                ckout << "Verification failed for " << name << ": row count "
                    << mdata.size() << " != " << rows << endl;
                CkAbort("Test failed");
            }
            for (std::size_t i = 0; i < rows; ++i) {
                if (mdata[i].size() != cols) {
                    ckout << "Verification failed for " << name << ": row " << i
                        << " col count " << mdata[i].size() << " != " << cols << endl;
                    CkAbort("Test failed");
                }
                for (std::size_t j = 0; j < cols; ++j) {
                    if (std::abs(mdata[i][j] - expected) > eps) {
                        ckout << "Verification failed for " << name << " at ("
                            << i << "," << j << "): expected " << expected
                            << " got " << mdata[i][j] << endl;
                        CkAbort("Test failed");
                    }
                }
            }
        };

        double start = CkWallTimer();

        ct::matrix mat1{mat_row_1, mat_col_1, 1};
        ct::matrix mat2{mat_row_1, mat_col_1, 1.5};
        ct::matrix mat3{mat_row_1, mat_col_1, .5};

        ct::matrix mat4 = mat1 + mat2 - mat3; // should be 2.0

        ct::matrix mat11{mat_row_2, mat_col_2, 1};
        ct::matrix mat12{mat_row_2, mat_col_2, 1.5};
        ct::matrix mat13{mat_row_2, mat_col_2, .5};

        ct::matrix mat14 = mat11 + mat12 - mat13; // should be 2.0

        ct::matrix mat111{mat_row_3, mat_col_3, 1};
        ct::matrix mat112{mat_row_3, mat_col_3, 1.5};
        ct::matrix mat113{mat_row_3, mat_col_3, .5};

        ct::matrix mat114 = mat111 + mat112 - mat113; // should be 2.0

        ct::sync();

        double end = CkWallTimer();
        ckout << "Execution Time (Phase 1): " << end - start << endl;
        ckout << "Running Correctness checks for (Phase 1)" << endl;

        // --- Verification after Phase 1 ---
        {
            auto mat1_data   = mat1.get();
            auto mat2_data   = mat2.get();
            auto mat3_data   = mat3.get();
            auto mat4_data   = mat4.get();

            auto mat11_data  = mat11.get();
            auto mat12_data  = mat12.get();
            auto mat13_data  = mat13.get();
            auto mat14_data  = mat14.get();

            auto mat111_data = mat111.get();
            auto mat112_data = mat112.get();
            auto mat113_data = mat113.get();
            auto mat114_data = mat114.get();

            // Verify values for Phase 1 matrices
            verify_mat(mat1_data,   mat_row_1, mat_col_1, 1.0,  "mat1");
            verify_mat(mat2_data,   mat_row_1, mat_col_1, 1.5,  "mat2");
            verify_mat(mat3_data,   mat_row_1, mat_col_1, 0.5,  "mat3");
            verify_mat(mat4_data,   mat_row_1, mat_col_1, 2.0,  "mat4"); // 1 + 1.5 - 0.5

            verify_mat(mat11_data,  mat_row_2, mat_col_2, 1.0,  "mat11");
            verify_mat(mat12_data,  mat_row_2, mat_col_2, 1.5,  "mat12");
            verify_mat(mat13_data,  mat_row_2, mat_col_2, 0.5,  "mat13");
            verify_mat(mat14_data,  mat_row_2, mat_col_2, 2.0,  "mat14"); // 1 + 1.5 - 0.5

            verify_mat(mat111_data, mat_row_3, mat_col_3, 1.0,  "mat111");
            verify_mat(mat112_data, mat_row_3, mat_col_3, 1.5,  "mat112");
            verify_mat(mat113_data, mat_row_3, mat_col_3, 0.5,  "mat113");
            verify_mat(mat114_data, mat_row_3, mat_col_3, 2.0,  "mat114"); // 1 + 1.5 - 0.5
        }

        ckout << "[SUCCESS] Correctness checks passed for (Phase 1)" << endl;

        start = CkWallTimer();
        mat4 = mat1 - mat3 + mat4;        // 1 - 0.5 + 2.0 = 2.5
        mat14 = mat11 - mat13 + mat14;    // 2.5
        mat114 = mat111 - mat113 + mat114; // 2.5

        // in-place operation
        ct::matrix mat115 = mat112 -= mat113; // mat112: 1.5 - 0.5 = 1.0, mat115 = 1.0
        mat115 += mat111 += mat112;           // mat111: 1.0 + 1.0 = 2.0, mat115: 1.0 + 2.0 = 3.0

        ct::sync();

        end = CkWallTimer();
        ckout << "Execution Time (Phase 2): " << end - start << endl;
        ckout << "Running Correctness checks for (Phase 2)" << endl;

        // --- Verification after Phase 2 ---
        {
            auto mat4_data    = mat4.get();
            auto mat14_data   = mat14.get();
            auto mat114_data  = mat114.get();

            auto mat111_data  = mat111.get();
            auto mat112_data  = mat112.get();
            auto mat113_data  = mat113.get();
            auto mat115_data  = mat115.get();

            // mat4/mat14/mat114 expected 2.5 after mat - mat + prev
            verify_mat(mat4_data,   mat_row_1, mat_col_1, 2.5, "mat4");
            verify_mat(mat14_data,  mat_row_2, mat_col_2, 2.5, "mat14");
            verify_mat(mat114_data, mat_row_3, mat_col_3, 2.5, "mat114");

            // In-place results
            verify_mat(mat112_data, mat_row_3, mat_col_3, 1.0, "mat112"); // became 1.0
            verify_mat(mat111_data, mat_row_3, mat_col_3, 2.0, "mat111"); // became 2.0 after +=
            verify_mat(mat113_data, mat_row_3, mat_col_3, 0.5, "mat113"); // unchanged
            verify_mat(mat115_data, mat_row_3, mat_col_3, 3.0, "mat115"); // final value
        }

        ckout << "[SUCCESS] Correctness checks passed for (Phase 2)" << endl;

        start = CkWallTimer();

        // copy operator
        mat4 = mat1;            // mat4 -> 1.0
        // copy constructor
        ct::matrix mat5 = mat4; // mat5 -> 1.0

        ct::sync();

        end = CkWallTimer();
        ckout << "Execution Time (Phase 3): " << end - start << endl;

        ckout << "Running Correctness checks for (Phase 3)" << endl;

        // --- Verification after Phase 3 ---
        {
            auto mat4_data = mat4.get();
            auto mat5_data = mat5.get();
            // mat4 and mat5 must equal mat1 (1.0)
            verify_mat(mat4_data, mat_row_1, mat_col_1, 1.0, "mat4 (after copy)");
            verify_mat(mat5_data, mat_row_1, mat_col_1, 1.0, "mat5 (copy constructor)");
        }

        ckout << "[SUCCESS] Correctness checks passed for (Phase 3)" << endl;

        // start = CkWallTimer();

        // ct::matrix x{1 << 13, 1 << 13, 1.0};
        // ct::vector y{1 << 13, 2.0};
        // ct::vector x_dot_y = ct::dot(x, y);
        // ct::sync();

        // end = CkWallTimer();

        // ckout << "Execution Time (mat-vec dot product): " << end - start
        //       << endl;

        // start = CkWallTimer();

        // ct::scalar scal1 = ct::dot(x_dot_y, x_dot_y);
        // double underlying_val = scal1.get();
        // ckout << "[Result] Dot-product over resultant vector: "
        //       << underlying_val << endl;

        // Eigen::MatrixXd eX = Eigen::MatrixXd::Constant(1 << 11, 1 << 12, 1.);
        // Eigen::VectorXd ey = Eigen::VectorXd::Constant(1 << 12, 2.);
        // Eigen::VectorXd ex_y = eX * ey;

        // double eres = ex_y.dot(ex_y);
        // ckout << "[Result: Eigen] Dot-product over resultant vector: " << eres
        //       << endl;

        // end = CkWallTimer();
        // ckout << "Execution Time (vec-vec dot product): " << end - start
        //       << endl;

        // ckout << "[Result] MatMul over 2 matrices: " << eres << endl;
        // start = CkWallTimer();

        // ct::matrix m1{1 << 10, 1 << 10, 0};
        // ct::matrix m2{1 << 10, 1 << 10, 0};
        // ct::matrix m3 = m1 * m2;
        // ct::sync(m1.matrix_shape());

        // end = CkWallTimer();
        // ckout << "Execution Time (mat-mul): " << end - start << endl;

        // ct::vector v1{1 << 10, 0};
        // ct::vector vres = ct::dot(m3, v1);
        // ct::scalar sval = ct::dot(vres, vres);
        // double uval = sval.get();

        // ckout << "[Result] Mat-mul sum: " << uval << endl;

        ct::finalize();
        CkExit();
    }
};

#include "base.def.h"

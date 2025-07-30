#include <charmtyles/charmtyles.hpp>

#include "base.decl.h"

class sqrt_t : public ct::unary_operator
{
public:
    sqrt_t() = default;
    ~sqrt_t() {}

    using ct::unary_operator::unary_operator;

    inline double operator()(std::size_t index, double value) override final
    {
        return 1.0;
    }

    inline double operator()(
        std::size_t rows, std::size_t cols, double value) override final
    {
        return 0.0;
    }

    PUPable_decl(sqrt_t);
    sqrt_t(CkMigrateMessage* m)
      : ct::unary_operator(m)
    {
    }

    void pup(PUP::er& p) final
    {
        ct::unary_operator::pup(p);
    }
};

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
        std::shared_ptr<ct::unary_operator> sqrt_ = std::make_shared<sqrt_t>();
        constexpr std::size_t mat_row_1 = 1 << 11;
        constexpr std::size_t mat_col_1 = 1 << 11;

        constexpr std::size_t mat_row_2 = 1 << 12;
        constexpr std::size_t mat_col_2 = 1 << 11;

        constexpr std::size_t mat_row_3 = 1 << 11;
        constexpr std::size_t mat_col_3 = 1 << 12;

        double start = CkWallTimer();

        ct::matrix mat1{mat_row_1, mat_col_1};
        ct::matrix mat2{mat_row_1, mat_col_1, 1.5};
        ct::matrix mat3{mat_row_1, mat_col_1, .5};

        ct::matrix mat4 = mat1 + mat2 - mat3;

        ct::matrix mat11{mat_row_2, mat_col_2};
        ct::matrix mat12{mat_row_2, mat_col_2, 1.5};
        ct::matrix mat13{mat_row_2, mat_col_2, .5};

        ct::matrix mat14 = mat11 + mat12 - mat13;

        ct::matrix mat111{mat_row_3, mat_col_3};
        ct::matrix mat112{mat_row_3, mat_col_3, 1.5};
        ct::matrix mat113{mat_row_3, mat_col_3, .5};

        ct::matrix mat114 = mat111 + mat112 - mat113;
        // ct::mat_impl::mat_instr_queue_t& queue =
        //     CT_ACCESS_SINGLETON(ct::mat_impl::mat_instr_queue);
        // queue.print_instructions();

        ct::sync();

        double end = CkWallTimer();

        ckout << "Execution Time (Phase 1): " << end - start << endl;

        start = CkWallTimer();
        mat4 = mat1 - mat3 + mat4;
        mat14 = mat11 - mat13 + mat14;
        mat114 = mat111 - mat113 + mat114;

        ct::sync();
        end = CkWallTimer();

        ckout << "Execution Time (Phase 2): " << end - start << endl;

        // copy operator
        mat4 = mat1;
        // copy constructor
        ct::matrix mat5 = mat4;

        ct::sync();

        start = CkWallTimer();

        ct::matrix x{1 << 13, 1 << 13, 1.0};
        ct::vector y{1 << 13, 2.0};
        // y = ct::unary_expr(y, sqrt_);
        // x = ct::unary_expr(x, sqrt_);
        ct::vector x_dot_y = ct::dot(x, y);
        // ct::sync();

        end = CkWallTimer();

        ckout << "Execution Time (mat-vec dot product): " << end - start
              << endl;

        start = CkWallTimer();

        ct::scalar scal1 = ct::dot(x_dot_y, x_dot_y);
        double underlying_val = scal1.get();
        ckout << "[Result] Dot-product over resultant vector: "
              << underlying_val << endl;

        Eigen::MatrixXd eX = Eigen::MatrixXd::Constant(1 << 11, 1 << 12, 1.);
        Eigen::VectorXd ey = Eigen::VectorXd::Constant(1 << 12, 2.);
        Eigen::VectorXd ex_y = eX * ey;

        double eres = ex_y.dot(ex_y);
        ckout << "[Result: Eigen] Dot-product over resultant vector: " << eres
              << endl;

        end = CkWallTimer();
        ckout << "Execution Time (vec-vec dot product): " << end - start
              << endl;

        ckout << "[Result] MatMul over 2 matrices: " << eres << endl;
        start = CkWallTimer();

        ct::matrix m1{1 << 10, 1 << 10, 1};
        ct::matrix m2{1 << 10, 1 << 10, 1};
        ct::matrix m3 = m1 * m2;
        ct::sync(m1.matrix_shape());

        end = CkWallTimer();
        ckout << "Execution Time (mat-mul): " << end - start << endl;

        ct::vector v1{1 << 10, 1};
        ct::vector vres = ct::dot(m3, v1);
        ct::scalar sval = ct::dot(vres, vres);
        double uval = sval.get();

        ckout << "[Result] Mat-mul sum: " << uval << endl;

        CkExit();
    }
};

#include "base.def.h"

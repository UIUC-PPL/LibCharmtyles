#include <charmtyles/charmtyles.hpp>

#include "jacobi.decl.h"

#include <Eigen/Dense>

/* readonly */ int DIMENSION;

class non_diag_generator : public ct::generator
{
public:
    non_diag_generator() = default;
    ~non_diag_generator() {}

    using ct::generator::generator;

    double generate(int row_id, int col_id) final
    {
        if (row_id != col_id)
            return (row_id + col_id) % 10;

        return 0;
    }

    double generate(int dimX) final
    {
        return dimX;
    }

    PUPable_decl(non_diag_generator);
    non_diag_generator(CkMigrateMessage* m)
      : ct::generator(m)
    {
    }

    void pup(PUP::er& p) final
    {
        ct::generator::pup(p);
    }
};

class diag_generator : public ct::generator
{
public:
    diag_generator() = default;
    ~diag_generator() {}

    using ct::generator::generator;

    double generate(int dimX) final
    {
        return 2 * dimX;
    }

    PUPable_decl(diag_generator);
    diag_generator(CkMigrateMessage* m)
      : ct::generator(m)
    {
    }

    void pup(PUP::er& p) final
    {
        ct::generator::pup(p);
    }
};

class Main : public CBase_Main
{
public:
    Main(CkArgMsg* msg)
    {
        DIMENSION = 1 << 14;
        if (msg->argc > 1)
            DIMENSION = atoi(msg->argv[1]);

        ct::init();
        thisProxy.benchmark(DIMENSION);
    }

    void benchmark(int dim)
    {
        std::shared_ptr<non_diag_generator> new_non_diag_generator =
            std::make_shared<non_diag_generator>();
        std::shared_ptr<diag_generator> new_diag_generator =
            std::make_shared<diag_generator>();

        ct::matrix R{dim, dim, new_non_diag_generator};
        ct::vector b{dim, new_non_diag_generator};
        ct::vector x{dim, new_non_diag_generator};
        ct::vector d{dim, new_diag_generator};

        ct::sync(R.matrix_shape());
        ct::sync(d.vector_shape());

        double start = CkWallTimer();

        ct::vector dot_r_x = ct::dot(R, x);
        x = (b - dot_r_x) / d;

        for (int i = 1; i != 100; ++i)
        {
            dot_r_x = ct::dot(R, x);
            x = (b - dot_r_x) / d;
        }
        ct::sync();

        double end = CkWallTimer();
        ct::scalar sum_x = ct::sum(x);
        ckout << "[Charmtyles] Execution Time: " << end - start << endl;
        ckout << "[Charmtyles] Squared Norm (x): " << sum_x.get() << endl;

        Eigen::ArrayXXd eR{dim, dim};
        Eigen::ArrayXd eb{dim};
        Eigen::ArrayXd ex{dim};
        Eigen::ArrayXd ed{dim};

        for (int i = 0; i != dim; ++i)
            for (int j = 0; j != dim; ++j)
                if (i != j)
                    eR(i, j) = (i + j) % 10;
                else
                    eR(i, j) = 0;

        for (int i = 0; i != dim; ++i)
        {
            eb(i) = i;
            ex(i) = i;
            ed(i) = 2 * i;
        }

        start = CkWallTimer();

        for (int i = 0; i != 100; ++i)
        {
            auto dot_r_x = ex * eR;
            ex = (eb - dot_r_x) / ed;
        }

        end = CkWallTimer();
        ckout << "[Eigen] Execution Time: " << end - start << endl;
        ckout << "[Eigen] Squared Norm (x): " << ex.sum() << endl;

        CkExit();
    }
};

#include "jacobi.def.h"

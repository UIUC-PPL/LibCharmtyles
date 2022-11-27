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
        int array_len = 1 << 20;
        int mat_dim = 1 << 10;

        DIMENSION = 1 << 14;
        if (msg->argc > 1)
            DIMENSION = atoi(msg->argv[1]);

        if (msg->argc > 2)
        {
            array_len = atoi(msg->argv[2]);
            mat_dim = atoi(msg->argv[3]);
        }

        ct::init(array_len, mat_dim);
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

        CkExit();
    }
};

#include "jacobi.def.h"

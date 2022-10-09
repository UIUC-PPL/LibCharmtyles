#include <charmtyles/charmtyles.hpp>

#include "base.decl.h"

class Main : public CBase_Main
{
public:
    Main(CkArgMsg* msg)
    {
        int dim = 1 << 14;
        if (msg->argc > 1)
            dim = atoi(msg->argv[1]);

        ct::init();
        thisProxy.benchmark(dim);
    }

    void benchmark(int dim)
    {
        ckout << "Matrix Dimensions: " << dim << "x" << dim << endl;
        ckout << "Vector Dimensions: " << dim << endl;

        ct::vec_impl::vec_instr_queue_t& vec_queue =
            CT_ACCESS_SINGLETON(ct::vec_impl::vec_instr_queue);
        ct::mat_impl::mat_instr_queue_t& mat_queue =
            CT_ACCESS_SINGLETON(ct::mat_impl::mat_instr_queue);

        ct::matrix A{dim, dim, 1.};

        // Random b, x
        ct::vector b{dim, 0.1};
        ct::vector x{dim, 0.5};

        ct::sync();

        double start = CkWallTimer();

        ct::vector a_x = ct::dot(A, x);
        ct::scalar dax = ct::dot(a_x, a_x);
        ct::vector r = b - a_x;
        ct::vector p = r;

        ct::scalar rsold = ct::dot(r, r);
        // ckout << "[-1] Rsold: " << rsold.get() << endl;

        double gres = 0.;
        ct::vector Ap = ct::dot(A, p);
        ct::scalar pAp = ct::dot(p, Ap);
        double rsold_value = rsold.get();
        double alpha = rsold_value / pAp.get();

        x = ct::axpy(alpha, p, x);
        r = ct::axpy(-alpha, Ap, r);

        ct::scalar rsnew = ct::dot(r, r);
        double rsnew_value = rsnew.get();

        if (std::sqrt(rsnew_value) < 1E-8)
            ckout << "Converged in 0 iterations" << endl;

        p = ct::axpy(rsnew_value / rsold_value, p, r);
        rsold = rsnew;

        // ckout << "[0] Rsold: " << rsold.get() << endl;

        for (int i = 1; i < 100; ++i)
        {
            Ap = ct::dot(A, p);
            pAp = ct::dot(p, Ap);
            rsold_value = rsold.get();
            alpha = rsold_value / pAp.get();

            x = ct::axpy(alpha, p, x);
            r = ct::axpy(-alpha, Ap, r);

            rsnew = ct::dot(r, r);
            rsnew_value = rsnew.get();

            if (std::sqrt(rsnew_value) < 1E-8)
            {
                ckout << "Converged in " << i << " iterations" << endl;
                break;
            }

            p = ct::axpy(rsnew_value / rsold_value, p, r);
            rsold = rsnew;

            // ckout << "[" << i << "] Rsold: " << rsold.get() << endl;
        }

        ct::sync();
        double end = CkWallTimer();

        ckout << "Execution Time: " << end - start << endl;
        ckout << "Value: " << rsold.get() << endl;

        CkExit();
    }
};

#include "base.def.h"

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

        Eigen::MatrixXd A = Eigen::MatrixXd::Constant(dim, dim, 1.);
        Eigen::VectorXd b = Eigen::VectorXd::Constant(dim, 2.);

        double start = CkWallTimer();
        Eigen::VectorXd res = A * b;
        double end = CkWallTimer();

        ckout << "Execution Time: " << end - start << endl;

        CkExit();
    }
};

#include "base.def.h"

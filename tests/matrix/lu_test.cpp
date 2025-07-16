#include <charmtyles/charmtyles.hpp>
#include "lu_test.decl.h"

/*readonly*/ CProxy_Main main_proxy;

class unit_vector_generator : public ct::generator
{
public:
    int unit_index;
    
    unit_vector_generator(int idx) : unit_index(idx) {}
    ~unit_vector_generator() {}

    using ct::generator::generator;

    double generate(int row_id, int col_id) final
    {
        return 0.0; // Not used for vectors
    }

    double generate(int dimX) final
    {
        return (dimX == unit_index) ? 1.0 : 0.0;
    }

    PUPable_decl(unit_vector_generator);
    unit_vector_generator(CkMigrateMessage* m)
      : ct::generator(m), unit_index(0)
    {
    }

    void pup(PUP::er& p) final
    {
        ct::generator::pup(p);
        p | unit_index;
    }
};

class test_matrix_generator : public ct::generator
{
public:
    test_matrix_generator() = default;
    ~test_matrix_generator() {}

    using ct::generator::generator;

    double generate(int row_id, int col_id) final
    {
        if (row_id == 0 && col_id == 0) return 4.0;
        if (row_id == 0 && col_id == 1) return 3.0;
        if (row_id == 1 && col_id == 0) return 6.0;
        if (row_id == 1 && col_id == 1) return 3.0;
        return 0.0;
    }

    double generate(int dimX) final
    {
        return dimX + 1.0;
    }

    PUPable_decl(test_matrix_generator);
    test_matrix_generator(CkMigrateMessage* m)
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
        main_proxy = thisProxy;
        ct::init();
        thisProxy.benchmark();
    }

    void benchmark()
    {
        std::shared_ptr<test_matrix_generator> gen = std::make_shared<test_matrix_generator>();
        ct::matrix A(2, 2, gen);
        
        CkPrintf("Matrix A = [[4,3], [6,3]]\n");
        CkPrintf("Matrix A sum: %f (expected: 16.0)\n", ct::sum(A).get());
        
        ct::matrix L = ct::get_L(A);
        ct::matrix U = ct::get_U(A);
        ct::matrix P = ct::get_P(A);

        CkPrintf("LU decomposition completed\n");
        CkPrintf("P sum: %f\n", ct::sum(P).get());
        CkPrintf("L sum: %f\n", ct::sum(L).get());
        CkPrintf("U sum: %f\n", ct::sum(U).get());

        

        CkPrintf("\nExpected values for verification:\n");
        CkPrintf("Expected P = [[0,1], [1,0]], sum = 2.0\n");
        CkPrintf("Expected L = [[1.0, 0.0], [0.6667, 1.0]], sum = 2.6667\n");
        CkPrintf("Expected U = [[6,3], [0,1]], sum = 10.0\n");

        CkExit();
    }
};

#include "lu_test.def.h"

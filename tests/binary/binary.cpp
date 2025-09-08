
#include <charmtyles/charmtyles.hpp>

#include "base.decl.h"

class multiply_t : public ct::binary_operator
{
public:
    multiply_t() = default;
    ~multiply_t() {}

    using ct::binary_operator::binary_operator;

    virtual double operator()(std::size_t index, double lhs, double rhs) final
    {
        // Element-wise multiplication
        return lhs * rhs;
    }

    PUPable_decl(multiply_t);
    multiply_t(CkMigrateMessage* m)
      : ct::binary_operator(m)
    {
    }

    void pup(PUP::er& p) final
    {
        ct::binary_operator::pup(p);
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
        ct::vector vec1(10, 2.0);
        ct::vector vec2(10, 3.0);

        std::shared_ptr<ct::binary_operator> multiply_op =
            std::make_shared<multiply_t>();

        ct::vector vec3 = ct::binary_expr(vec1, vec2, multiply_op);

        ct::scalar result = ct::max(vec3);
        double max_val = result.get();

        ckout << "Vector 1: all elements = 2.0" << endl;
        ckout << "Vector 2: all elements = 3.0" << endl;
        ckout << "Result of element-wise multiplication (max value): "
              << max_val << endl;
        ckout << "Expected result: 6.0" << endl;

        if (std::abs(max_val - 6.0) < 1e-10)
        {
            ckout << "Binary operator test PASSED!" << endl;
        }
        else
        {
            ckout << "Binary operator test FAILED!" << endl;
        }

        CkExit();
    }
};

#include "base.def.h"

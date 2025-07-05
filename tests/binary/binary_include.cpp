#include <charmtyles/charmtyles.hpp>
#include <charmtyles/frontend/basic_unary_operators.hpp>
#include <charmtyles/frontend/basic_binary_operators.hpp>

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
        
        ct::vector vec1(5, 3.0);  // [3, 3, 3, 3, 3]
        
        // Negation
        ct::vector neg_result = ct::unary_expr(vec1, ct::unary_ops::negate());
        ct::scalar neg_sum = ct::sum(neg_result);
        double neg_val = neg_sum.get();
        ckout << "   Negate [3,3,3,3,3] -> sum = " << neg_val << " (expected: -15)" << endl;
        
        ct::vector vec_neg(4, -2.5);  // [-2.5, -2.5, -2.5, -2.5]
        ct::vector abs_result = ct::unary_expr(vec_neg, ct::unary_ops::abs());
        ct::scalar abs_sum = ct::sum(abs_result);
        double abs_val = abs_sum.get();
        ckout << "sum = " << abs_val << " (expected: 10)" << endl;
        
        
        ct::vector vec3(5, 1.0);  // [1, 1, 1, 1, 1]
        ct::vector vec4(5, 2.0);  // [2, 2, 2, 2, 2]

        // Addition
        ct::vector add_result = ct::binary_expr(vec3, vec4, ct::binary_ops::add());
        ct::scalar add_sum = ct::sum(add_result);
        double add_val = add_sum.get();
        ckout << "sum = " << add_val << " (expected: 15)" << endl;
        
        
        
        
        CkExit();
    }
};

#include "base.def.h"

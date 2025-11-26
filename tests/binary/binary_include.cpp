#include <charmtyles/charmtyles.hpp>
#include <charmtyles/frontend/basic_unary_operators.hpp>
#include <charmtyles/frontend/basic_binary_operators.hpp>

#include "base.decl.h"

class Main : public CBase_Main
{
public:
    Main(CkArgMsg* msg)
    {
        ct::init();
        thisProxy.benchmark();
    }

    void benchmark()
    {
        auto start = CkWallTimer();

        ct::matrix x{21, 21, 1.0};
        ct::vector y{21, 1.0};
        ct::vector x_dot_y = ct::dot(y, x);
        ct::sync();

        auto end = CkWallTimer();

        ckout << "Execution Time (mat-vec dot product): " << end - start
              << endl;

        std::vector<double> val = x_dot_y.get();
        for(int i = 0; i < 40; i++) {
            ckout << val[i] << " ";
        }
        ckout << endl;
        // ct::vector vec1(5, 3.0);  // [3, 3, 3, 3, 3]
        
        // // Negation
        // ct::vector neg_result = ct::unary_expr(vec1, ct::unary_ops::negate());
        // ct::scalar neg_sum = ct::sum(neg_result);
        // double neg_val = neg_sum.get();
        // ckout << "Negate [3,3,3,3,3] -> sum = " << neg_val << " (expected: -15)" << endl;

        // ct::vector vec_neg(4, -2.5);  // [-2.5, -2.5, -2.5, -2.5]
        // ct::vector abs_result = ct::unary_expr(vec_neg, ct::unary_ops::abs());
        // ct::scalar abs_sum = ct::sum(abs_result);
        // double abs_val = abs_sum.get();
        // ckout << "sum = " << abs_val << " (expected: 10)" << endl;

        // ct::vector vec_scale(4, 2.5);  // [2.5, 2.5, 2.5, 2.5]
        // ct::vector scale_result = ct::unary_expr(vec_scale, ct::unary_ops::scale(2.0));
        // ct::scalar scale_sum = ct::sum(scale_result);
        // double scalar_result = scale_sum.get();
        // ckout << "sum = " << scalar_result << " (expected: 20)" << endl;
        
        // ct::vector vec3(5, 1.0);  // [1, 1, 1, 1, 1]
        // ct::vector vec4(5, 2.0);  // [2, 2, 2, 2, 2]

        // // Addition
        // ct::vector add_result = ct::binary_expr(vec3, vec4, ct::binary_ops::add());
        // ct::scalar add_sum = ct::sum(add_result);
        // double add_val = add_sum.get();
        // ckout << "sum = " << add_val << " (expected: 15)" << endl;

        ct::finalize();
        CkExit();
    }
};

#include "base.def.h"

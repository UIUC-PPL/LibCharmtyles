#include <charmtyles/charmtyles.hpp>
#include <vector>

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
        constexpr std::size_t vec_size_1 = 1 << 24;
        constexpr std::size_t vec_size_2 = 1 << 24;
        constexpr std::size_t vec_size_3 = 1 << 24;
        ct::vector vec1{vec_size_1, .5};
        // ct::vector vec2{vec_size_1, 1.5};
        // ct::vector vec3{vec_size_1, .5};
        // ct::vector vec11{vec_size_2, 0.0};
        // ct::vector vec12{vec_size_2, 1.5};
        // ct::vector vec13{vec_size_2, .5};
        // ct::vector vec111{vec_size_3, 0.0};
        // ct::vector vec112{vec_size_3, 1.5};
        // ct::vector vec113{vec_size_3, .5};

        // ct::vector vec4 = vec1 + vec2;    

        ct::vector exp_val = vec1;  
        // for(int i=0;i<10;i++){
        exp_val = ct::unary_expr(ct::unary_expr(ct::unary_expr(ct::unary_expr(ct::unary_expr(exp_val, ct::unary_ops::scale(10)), ct::unary_ops::scale(10)), ct::unary_ops::scale(10)), ct::unary_ops::scale(10)), ct::unary_ops::scale(10));
        //  = ct::unary_expr(exp_val, ct::unary_ops::scale(10));
        // }
        ct::sync();

        ckout<<"PRINT exp result"<<endl;
        auto vec1_data = exp_val.get();
        for(int i=0;i<10;i++){
           ckout<<vec1_data[i]<<endl;
        }
        ckout<<endl;
        // ct::vector vec14 = vec11 + vec12 - vec13;
        // ct::vector vec114 = vec111 + vec112 - vec113;   
        // ct::sync();

        // double start = CkWallTimer();
        // for (int i = 0; i < 100; i++) {
        //     vec4 = vec1 + vec2;    
        //     vec14 = vec11 + vec12 - vec13;
        //     vec114 = vec111 + vec112 - vec113;        
        // }
        // ct::sync();
        // double end = CkWallTimer();


        // ckout << "Execution Time (Phase 1): " << end - start << endl;

        // ckout << "Running correctness checks for phase 1" << endl;
        // // Retrieve values for all vectors once
        // auto vec1_data = vec1.get();
        // auto vec2_data = vec2.get();
        // auto vec3_data = vec3.get();
        // auto vec4_data = vec4.get();

        // auto vec11_data = vec11.get();
        // auto vec12_data = vec12.get();
        // auto vec13_data = vec13.get();
        // auto vec14_data = vec14.get();

        // auto vec111_data = vec111.get();
        // auto vec112_data = vec112.get();
        // auto vec113_data = vec113.get();
        // auto vec114_data = vec114.get();

        // // Verify values for all vectors
        // if (!(vec1_data.size() == vec_size_1 && std::all_of(vec1_data.begin(), vec1_data.end(), [](double v) { return v == 0.5; }))) {
        //     ckout << "Verification failed for vec1_data" << endl;
        //     CkAbort("Test failed");
        // }
        // if (!(vec2_data.size() == vec_size_1 && std::all_of(vec2_data.begin(), vec2_data.end(), [](double v) { return v == 1.5; }))) {
        //     ckout << "Verification failed for vec2_data" << endl;
        //     CkAbort("Test failed");
        // }
        // if (!(vec3_data.size() == vec_size_1 && std::all_of(vec3_data.begin(), vec3_data.end(), [](double v) { return v == 0.5; }))) {
        //     ckout << "Verification failed for vec3_data" << endl;
        //     CkAbort("Test failed");
        // }
        // if (!(vec4_data.size() == vec_size_1 && std::all_of(vec4_data.begin(), vec4_data.end(), [](double v) { return v == 2.0; }))) {
        //     ckout << "Verification failed for vec4_data" << endl;
        //     CkAbort("Test failed");
        // }

        // if (!(vec11_data.size() == vec_size_2 && std::all_of(vec11_data.begin(), vec11_data.end(), [](double v) { return v == 0.0; }))) {
        //     ckout << "Verification failed for vec11_data" << endl;
        //     CkAbort("Test failed");
        // }
        // if (!(vec12_data.size() == vec_size_2 && std::all_of(vec12_data.begin(), vec12_data.end(), [](double v) { return v == 1.5; }))) {
        //     ckout << "Verification failed for vec12_data" << endl;
        //     CkAbort("Test failed");
        // }
        // if (!(vec13_data.size() == vec_size_2 && std::all_of(vec13_data.begin(), vec13_data.end(), [](double v) { return v == 0.5; }))) {
        //     ckout << "Verification failed for vec13_data" << endl;
        //     CkAbort("Test failed");
        // }
        // if (!(vec14_data.size() == vec_size_2 && std::all_of(vec14_data.begin(), vec14_data.end(), [](double v) { return v == 1.0; }))) {
        //     ckout << "Verification failed for vec14_data" << endl;
        //     CkAbort("Test failed");
        // }

        // if (!(vec111_data.size() == vec_size_3 && std::all_of(vec111_data.begin(), vec111_data.end(), [](double v) { return v == 0.0; }))) {
        //     ckout << "Verification failed for vec111_data" << endl;
        //     CkAbort("Test failed");
        // }
        // if (!(vec112_data.size() == vec_size_3 && std::all_of(vec112_data.begin(), vec112_data.end(), [](double v) { return v == 1.5; }))) {
        //     ckout << "Verification failed for vec112_data" << endl;
        //     CkAbort("Test failed");
        // }
        // if (!(vec113_data.size() == vec_size_3 && std::all_of(vec113_data.begin(), vec113_data.end(), [](double v) { return v == 0.5; }))) {
        //     ckout << "Verification failed for vec113_data" << endl;
        //     CkAbort("Test failed");
        // }
        // if (!(vec114_data.size() == vec_size_3 && std::all_of(vec114_data.begin(), vec114_data.end(), [](double v) { return v == 1.0; }))) {
        //     ckout << "Verification failed for vec114_data" << endl;
        //     CkAbort("Test failed");
        // }

        // ckout << "[SUCCESS] All Phase 1 Tests passed" << endl;

        // ct::scalar halo{42.0};
        // ct::scalar illo{27.0};
        // ct::scalar reso = halo + illo;
        // ct::sync();
        // double reso_h = reso.get();
        // ckout << "RESO> " << reso_h << endl;

        // start = CkWallTimer();
        // vec4 = vec1 - vec3 + vec4;
        // vec14 = vec11 - vec13 + vec14;
        // vec114 = vec111 - vec113 + vec114;

        // // in-place operation
        // ct::vector vec115 = vec112 -= vec113;
        // vec115 += vec111 += vec112;

        // ct::sync();
        // end = CkWallTimer();

        // ckout << "Execution Time (Phase 2): " << end - start << endl;

        // ckout << "Running correctness checks for phase 2" << endl;

        // // Retrieve values after operations
        // vec4_data = vec4.get();
        // vec14_data = vec14.get();
        // vec114_data = vec114.get();
        // auto vec115_data = vec115.get();

        // // Verify values after operations
        // if (!(vec4_data.size() == vec_size_1 && std::all_of(vec4_data.begin(), vec4_data.end(), [](double v) { return v == 2.0; }))) {
        //     ckout << "Verification failed for vec4_data after operations" << endl;
        //     CkAbort("Test failed");
        // }
        // if (!(vec14_data.size() == vec_size_2 && std::all_of(vec14_data.begin(), vec14_data.end(), [](double v) { return v == 0.5; }))) {
        //     ckout << "Verification failed for vec14_data after operations" << endl;
        //     CkAbort("Test failed");
        // }
        // if (!(vec114_data.size() == vec_size_3 && std::all_of(vec114_data.begin(), vec114_data.end(), [](double v) { return v == 0.5; }))) {
        //     ckout << "Verification failed for vec114_data after operations" << endl;
        //     CkAbort("Test failed");
        // }
        // if (!(vec115_data.size() == vec_size_3 && std::all_of(vec115_data.begin(), vec115_data.end(), [](double v) { return v == 2.0; }))) {
        //     ckout << "Verification failed for vec115_data after operations" << endl;
        //     CkAbort("Test failed");
        // }

        // ckout << "[SUCCESS] All Phase 2 Tests passed" << endl;

        // start = CkWallTimer();

        // // copy operator
        // vec4 = vec1;
        // // copy constructor
        // ct::vector vec5 = vec4;

        // ct::sync();

        // end = CkWallTimer();

        // ckout << "Execution Time (Phase 3): " << end - start << endl;

        // ckout << "Running correctness checks for phase 3" << endl;

        // // Retrieve copied values
        // vec4_data = vec4.get();
        // auto vec5_data = vec5.get();

        // // Verify copied values
        // if (!(vec4_data.size() == vec_size_1 && std::all_of(vec4_data.begin(), vec4_data.end(), [](double v) { return v == 0.5; }))) {
        //     ckout << "Verification failed for vec4_data after copy" << endl;
        //     CkAbort("Test failed");
        // }
        // if (!(vec5_data.size() == vec_size_1 && std::all_of(vec5_data.begin(), vec5_data.end(), [](double v) { return v == 0.5; }))) {
        //     ckout << "Verification failed for vec5_data after copy" << endl;
        //     CkAbort("Test failed");
        // }

        // ckout << "[SUCCESS] All Phase 3 Tests passed" << endl;

        // ct::vector x{1 << 21, 1.0};
        // ct::vector y{1 << 21, 2.0};
        // ct::scalar scal1 = ct::dot(x, y);
        // double underlying_val = scal1.get();

        // ckout << "Result of Vector dot product: " << underlying_val << endl;

        // // Test our new from_vector functionality
        // std::vector<double> test_data = {1.1, 2.2, 3.3, 4.4, 5.5};
        // ct::vector custom_vec = ct::from_vector(test_data);
        // ct::sync();
        
        // std::vector<double> result = custom_vec.get();
        // ckout << "Custom vector elements: ";
        // for (const auto& val : result) {
        //     ckout << val << " ";
        // }
        // ckout << endl;

        ct::finalize();
        CkExit();
    }
};

#include "base.def.h"

#include <charmtyles/charmtyles.hpp>

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
        // basic functionality
        std::vector<double> vec = {1,2,3,4,5,6};
        ct::vector ctvec = ct::from_vector(vec);
        ct::sync();
        
        ckout << "Original vector: [1,2,3,4,5,6]" << endl;
        ckout << "Testing with k=3 chunks: [1,2], [3,4], [5,6]" << endl;
        
        // original get(k) function
        std::vector<double> get3vec = ctvec.get(3);
        ckout << "get(3) result: ";
        for (int i = 0; i < 3; i++) {
            ckout << get3vec[i] << " ";
        }
        ckout << endl;

        // new chunk-based functions
        ct::vector avg_result = ct::get_avg(ctvec, 3);
        ct::vector min_result = ct::get_min(ctvec, 3);  
        ct::vector max_result = ct::get_max(ctvec, 3);
        
        ct::sync();
        
        std::vector<double> get3avg = avg_result.get();
        std::vector<double> get3min = min_result.get();
        std::vector<double> get3max = max_result.get();

        ckout << "Chunk averages (expected [1.5, 3.5, 5.5]): ";
        for (int i = 0; i < 3; i++) {
            ckout << get3avg[i] << " ";
        }
        ckout << endl;

        ckout << "Chunk minimums (expected [1, 3, 5]): ";
        for (int i = 0; i < 3; i++) {
            ckout << get3min[i] << " ";
        }
        ckout << endl;

        ckout << "Chunk maximums (expected [2, 4, 6]): ";
        for (int i = 0; i < 3; i++) {
            ckout << get3max[i] << " ";
        }
        ckout << endl;
        
        CkExit();
    }
};

#include "base.def.h"

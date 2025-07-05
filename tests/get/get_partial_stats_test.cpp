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

        // Test 1: Basic get_avg function
        ckout << "1. Testing get_avg function:" << endl;
        
        ct::vector vec1(10, 5.0);
        std::vector<double> veck = vec1.get(2);
        for (double val : veck) {
            ckout << val << " ";
        }
        ckout << endl;
        ct::scalar avg_result = ct::get_avg(vec1, 3);
        double avg_value = avg_result.get();
        
        ckout << "   Created vector of size 10 with all elements = 5.0" << endl;
        ckout << "   Average result: " << avg_value << endl;
        

        CkExit();
    }
};

#include "base.def.h"

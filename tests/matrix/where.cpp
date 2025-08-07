#include <charmtyles/charmtyles.hpp>

#include "where.decl.h"

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
        ct::matrix orig{1000, 1000, 1.0};
        ct::matrix cpy{1000, 1000, 2.0};
        std::vector<std::vector<double>> vec = ct::matrix(
            ct::where(orig, cpy, !((orig + 2. >= orig) && (orig + 2. < orig))) +
            3.2)
                                                   .get();
        std::cout << "The vector is: ";
        for (size_t i = 0; i < vec.size(); i++)
        {
            std::cout << vec[i][i] << " ";
        }
        std::cout << std::endl;

        CkExit();
    }
};

#include "where.def.h"

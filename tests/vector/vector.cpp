#include <charmtyles/charmtyles.hpp>
#include <vector>

#include "base.decl.h"
#include <Kokkos_Core.hpp>

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
        constexpr std::size_t vec_size_1 = 1 << 20;
        constexpr std::size_t vec_size_2 = 1 << 26;
        constexpr std::size_t vec_size_3 = 1 << 22;

        double start = CkWallTimer();

        ct::vector vec1{vec_size_1, .5};
        ct::vector vec2{vec_size_1, 1.5};
        ct::vector vec3{vec_size_1, .5};

        ct::vector vec4 = ((vec1 * vec3) + vec2 + (vec2 - vec3) * vec1);

        std::vector<double> res = vec4.get();
        for(int i = 0; i < 10; i ++) {
            ckout << res[i] << " ";
        }
        ckout << endl;
        ct::finalize();
        CkExit();
    }
};

#include "base.def.h"

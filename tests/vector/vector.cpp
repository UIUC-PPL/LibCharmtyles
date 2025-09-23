#include <charmtyles/charmtyles.hpp>
#include <vector>

#include "base.decl.h"
#include <Kokkos_Core.hpp>

class Main : public CBase_Main
{
public:
    Main(CkArgMsg* msg)
    {
        {
              Kokkos::initialize();
  {
    Kokkos::View<int*> a("v", 5);
    Kokkos::View<int*> b("v", 5);
    Kokkos::View<int*> c("v", 5);

    Kokkos::parallel_for("fill", 5, KOKKOS_LAMBDA(int i) { a(i) = 2; });
    Kokkos::parallel_for("fill", 5, KOKKOS_LAMBDA(int i) { b(i) = 3; });
    Kokkos::parallel_for("fill", 5, KOKKOS_LAMBDA(int i) { c(i) = a(i) + b(i); });
    auto chost = Kokkos::create_mirror_view_and_copy(Kokkos::HostSpace(), c);
    KOKKOS_ASSERT(chost(0) == 5);

    Kokkos::View<int**> a_("a", 5, 5);
    Kokkos::View<int**> b_("b", 5, 5);
    Kokkos::View<int**> c_("c", 5, 5);

    Kokkos::parallel_for(
      "fill_a",
      Kokkos::MDRangePolicy<Kokkos::Rank<2>>({0,0}, {5,5}),
      KOKKOS_LAMBDA(int i, int j) {
        a_(i, j) = 2;
      });

    Kokkos::parallel_for(
      "fill_b",
      Kokkos::MDRangePolicy<Kokkos::Rank<2>>({0,0}, {5,5}),
      KOKKOS_LAMBDA(int i, int j) {
        b_(i, j) = 3;
      });

    Kokkos::parallel_for(
      "compute_c",
      Kokkos::MDRangePolicy<Kokkos::Rank<2>>({0,0}, {5,5}),
      KOKKOS_LAMBDA(int i, int j) {
        c_(i, j) = a_(i, j) * b_(i, j);
      });

    auto c_host = Kokkos::create_mirror_view_and_copy(Kokkos::HostSpace(), c_);
    KOKKOS_ASSERT(c_host(0, 0) == 6);
  }
  Kokkos::printf("Goodbye World\n");
  Kokkos::finalize();
        }
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

        ct::vector vec4 = vec1 + vec2;

        std::vector<double> res = vec4.get();
        for(int i = 0; i < 10; i ++) {
            ckout << res[i] << " ";
        }
        ckout << endl;

        CkExit();
    }
};

#include "base.def.h"

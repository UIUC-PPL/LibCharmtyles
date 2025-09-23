set(CHARM_DIR "/home/shogo/master/Kale/charm/netlrts-linux-x86_64")
set(BASE_DIR "/home/shogo/master/Kale/charmTylesUpstream/LibCharmtyles")
set(EIGEN_DIR "/usr/include/eigen3")

set(CHARMC "${CHARM_DIR}/bin/charmc")
set(OPTS "-c++-option -std=c++17 -O3 -march=native -DNDEBUG")
set(LD_OPTS "")
set(INCS "-I${BASE_DIR}")

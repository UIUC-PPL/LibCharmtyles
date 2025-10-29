if [ -d "./kokkos/install" ]; then
    echo "Kokkos Found at ${PWD}/kokkos/install"
else
    git clone https://github.com/kokkos/kokkos.git
    cd kokkos
    mkdir build
    cd build
    cmake -DBUILD_SHARED_LIBS=ON ..
    make -j${nproc}
    cd ..
    mkdir install
    cmake --install build --prefix install
    cd ..
fi

if [ -d "./kokkos-kernels/install" ]; then
    echo "Kokkos Kernels Found at ${PWD}/kokkos-kernels/install"
else
    git clone https://github.com/kokkos/kokkos-kernels.git
    cd kokkos-kernels
    mkdir build
    cd build
    cmake .. -DCMAKE_CXX_COMPILER=g++ -DCMAKE_INSTALL_PREFIX=${PWD}/../install -DKokkos_ROOT=${PWD}/../../kokkos/install -DBUILD_SHARED_LIBS=ON
    make -j${nproc}
    cd .. 
    mkdir install
    cmake --install build --prefix install
    cd ..
fi
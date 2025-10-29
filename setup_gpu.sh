if [ -d "./kokkos/install" ]; then
    echo "Kokkos Found at ${PWD}/kokkos/install"
else
    git clone https://github.com/kokkos/kokkos.git
    cd kokkos
    rm -rf build
    mkdir build
    cd build
    pwd

    # ensure that you have cmake/3.27.9 cuda/12.4.0 and eigen[for later] loaded

    ## for delta please run these before running the setup script
    # module load cuda/12.4.0
    # module load eigen
    # module load cmake/3.27.9

    ## The best practice is to let cmake autodetect the architecture, please run on a GPU syster or add a srun
    srun cmake -DBUILD_SHARED_LIBS=ON .. -DKokkos_ENABLE_CUDA=ON
    pwd
    make -j${nproc}
    cd ..
    mkdir install
    cmake --install build --prefix install
    cd ..
fi

if [ -d "./kokkos-kernels/install" ]; then
    echo "Kokkos Kernels Found at ${PWD}/kokkos_kernels/install"
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

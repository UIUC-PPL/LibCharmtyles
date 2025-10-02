if [ -d "./kokkos/install" ]; then
    echo "Kokkos Found at ${PWD}/kokkos/install"
else
    git clone https://github.com/kokkos/kokkos.git
    cd kokkos
    git checkout 4.5.01
    mkdir build
    cd build
    cmake -DBUILD_SHARED_LIBS=ON ..
    make -j${nproc}
    cd ..
    mkdir install
    cmake --install build --prefix install
    cd ..
fi

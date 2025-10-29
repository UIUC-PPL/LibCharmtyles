if [ -d "./kokkos/install" ]; then
    echo "Kokkos Found at ${PWD}/kokkos/install"
else
    export KOKKOS_VERSION=4.7.01 # Replace with the actual version
    export KOKKOS_DOWNLOAD_URL=https://github.com/kokkos/kokkos/releases/download/${KOKKOS_VERSION}
    curl -sLO ${KOKKOS_DOWNLOAD_URL}/kokkos-${KOKKOS_VERSION}.tar.gz
    tar -xzvf kokkos-${KOKKOS_VERSION}.tar.gz
    rm kokkos-${KOKKOS_VERSION}.tar.gz
    mv kokkos-${KOKKOS_VERSION} kokkos
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
    export KOKKOS_KERNELS_VERSION=4.7.01 # Replace with the actual version
    export KOKKOS_DOWNLOAD_URL=https://github.com/kokkos/kokkos-kernels/releases/download/${KOKKOS_VERSION}
    curl -sLO ${KOKKOS_DOWNLOAD_URL}/kokkos-kernels-${KOKKOS_VERSION}.tar.gz
    tar -xzvf kokkos-kernels-${KOKKOS_VERSION}.tar.gz
    rm kokkos-kernels-${KOKKOS_VERSION}.tar.gz
    mv kokkos-kernels-${KOKKOS_VERSION} kokkos-kernels
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
cmake_minimum_required(VERSION 3.16)
project(charmTyles)

include(${CMAKE_SOURCE_DIR}/../../config.cmake)

set(Kokkos_ROOT ${KOKKOS_DIR})
find_package(Kokkos 4.5 REQUIRED CONFIG)

set(KokkosKernels_DIR ${KOKKOS_KERNELS_DIR}/lib/cmake/KokkosKernels)
find_package(KokkosKernels REQUIRED)

if(Charm_ENABLE_GPU)
    message(STATUS "Building for a GPU backend")
    add_definitions(-DGPU_BACKEND)
    add_definitions(-DCUDA_DIR=\"${CUDA_DIR}\")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GPU_OPTS} ${LD_OPTS}")
else()
    message(STATUS "Building for a CPU backend")
    set(CMAKE_CXX_COMPILER "${CHARMC}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CPU_OPTS} ${LD_OPTS}")
endif()

add_definitions(-DKOKKOS_DIR=\"${KOKKOS_DIR}\")

add_custom_command(
    OUTPUT  ${BASE_DIR}/charmtyles/backend/libcharmtyles.decl.h
    COMMAND ${CMAKE_COMMAND} -E chdir ${BASE_DIR}/charmtyles/backend
            ${CHARMC} ${BASE_DIR}/charmtyles/backend/charmtyles.ci
    DEPENDS ${BASE_DIR}/charmtyles/backend/charmtyles.ci
    COMMENT "Processing charmtyles ci files"
)

add_custom_command(
    OUTPUT  ${CMAKE_SOURCE_DIR}/base.decl.h
    COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_SOURCE_DIR}
            ${CHARMC} ${CMAKE_SOURCE_DIR}/base.ci
    DEPENDS ${CMAKE_SOURCE_DIR}/base.ci
    COMMENT "Processing base ci files"
)

if(Charm_ENABLE_GPU)
    add_library(vector OBJECT vector.cpp ${BASE_DIR}/charmtyles/backend/libcharmtyles.decl.h ${CMAKE_SOURCE_DIR}/base.decl.h)
    target_include_directories(vector PRIVATE ${BASE_DIR} ${BASE_DIR}/charmtyles/backend ${EIGEN_DIR}/include ${CHARM_DIR}/include)
    target_link_libraries(vector Kokkos::kokkos Kokkos::kokkoskernels)

    add_custom_command(
        OUTPUT "${CMAKE_BINARY_DIR}/vector.out"
        COMMAND ${CHARMC} ${GPU_LINK_OPTS} $<TARGET_OBJECTS:vector> -o ${CMAKE_BINARY_DIR}/vector.out
        DEPENDS vector
        COMMENT "Linking charm build against kokkos and cuda"
    )

    add_custom_target(result ALL
        DEPENDS "${CMAKE_BINARY_DIR}/vector.out"
    )
else()
    add_executable(vector.out vector.cpp ${BASE_DIR}/charmtyles/backend/libcharmtyles.decl.h ${CMAKE_SOURCE_DIR}/base.decl.h)
    target_include_directories(vector.out PRIVATE ${BASE_DIR} ${BASE_DIR}/charmtyles/backend ${EIGEN_DIR})
    target_link_libraries(vector.out Kokkos::kokkos Kokkos::kokkoskernels)
endif()

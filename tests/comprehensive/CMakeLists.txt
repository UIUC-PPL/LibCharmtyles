cmake_minimum_required(VERSION 3.16)
project(charmtyles_comprehensive_tests)

include(${CMAKE_SOURCE_DIR}/../../config.cmake)

set(Kokkos_ROOT ${KOKKOS_DIR})
find_package(Kokkos 4.5 REQUIRED CONFIG)

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

add_definitions(-DKOKKOS_DIR=\"${KOKKOS_DIR}\" )

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
    add_library(comprehensive_tests OBJECT comprehensive.cpp ${BASE_DIR}/charmtyles/backend/libcharmtyles.decl.h ${CMAKE_SOURCE_DIR}/base.decl.h)
    target_include_directories(comprehensive_tests PRIVATE ${BASE_DIR} ${BASE_DIR}/charmtyles/backend ${EIGEN_DIR}/include ${CHARM_DIR}/include)
    target_link_libraries(comprehensive_tests Kokkos::kokkos)

    add_custom_command(
        OUTPUT "${CMAKE_BINARY_DIR}/comprehensive_tests.out"
        COMMAND ${CHARMC} ${GPU_LINK_OPTS} $<TARGET_OBJECTS:comprehensive_tests> -o ${CMAKE_BINARY_DIR}/comprehensive_tests.out
        DEPENDS comprehensive_tests
        COMMENT "Linking charm build against kokkos and cuda"
    )

    add_custom_target(result ALL
        DEPENDS "${CMAKE_BINARY_DIR}/comprehensive_tests.out"
    )
else()
    add_executable(comprehensive_tests.out comprehensive.cpp ${BASE_DIR}/charmtyles/backend/libcharmtyles.decl.h ${CMAKE_SOURCE_DIR}/base.decl.h)
    target_include_directories(comprehensive_tests.out PRIVATE ${BASE_DIR} ${BASE_DIR}/charmtyles/backend ${EIGEN_DIR})
    target_link_libraries(comprehensive_tests.out Kokkos::kokkos)
endif()

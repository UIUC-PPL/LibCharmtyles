cmake_minimum_required(VERSION 3.16)
project(vector)

include(${CMAKE_SOURCE_DIR}/../../config.cmake)

include(FetchContent)
FetchContent_Declare(
  Kokkos
  URL https://github.com/kokkos/kokkos/archive/refs/tags/4.5.01.zip
)
FetchContent_MakeAvailable(Kokkos)

message(STATUS "CHARMC = ${CHARMC}")

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
    COMMENT "Processing vector ci files"
)

add_custom_command(
    OUTPUT  ${CMAKE_SOURCE_DIR}/vector.o
    COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_SOURCE_DIR}
            ${CHARMC} -I${EIGEN_DIR} -I${BASE_DIR} -I${BASE_DIR}/charmtyles/backend -I${CMAKE_BINARY_DIR}/_deps/kokkos-build -I${CMAKE_BINARY_DIR}/_deps/kokkos-src/core/src ${CMAKE_SOURCE_DIR}/vector.cpp
    DEPENDS ${CMAKE_SOURCE_DIR}/vector.cpp
)

add_executable(vector.out ${CMAKE_SOURCE_DIR}/vector.o)
target_link_libraries(vector.out Kokkos::kokkos)

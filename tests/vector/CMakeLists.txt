cmake_minimum_required(VERSION 3.16)
project(charmTyles)

include(${CMAKE_SOURCE_DIR}/../../config.cmake)

include(FetchContent)
FetchContent_Declare(
  Kokkos
  URL https://github.com/kokkos/kokkos/archive/refs/tags/4.5.01.zip
)
FetchContent_MakeAvailable(Kokkos)

message(STATUS "CHARMC = ${CHARMC}")

set(CMAKE_CXX_COMPILER "${CHARMC}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OPTS} ${LD_OPTS}")

add_custom_command(
    OUTPUT  ${BASE_DIR}/charmtyles/backend/libcharmtyles.decl.h
    COMMAND ${CMAKE_COMMAND} -E chdir ${BASE_DIR}/charmtyles/backend
            ${CHARMC} ${BASE_DIR}/charmtyles/backend/charmtyles.ci
    DEPENDS ${BASE_DIR}/charmtyles/backend/charmtyles.ci
    COMMENT "Processing charmtyles ci files"
)

add_executable(vector.out vector.cpp ${BASE_DIR}/charmtyles/backend/libcharmtyles.decl.h)
target_include_directories(vector.out PRIVATE ${BASE_DIR} ${BASE_DIR}/charmtyles/backend ${EIGEN_DIR})
target_link_libraries(vector.out Kokkos::kokkos)

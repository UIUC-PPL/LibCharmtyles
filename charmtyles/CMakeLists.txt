cmake_minimum_required(VERSION 3.16)
project(charmTyles)

include(${CMAKE_SOURCE_DIR}/../config.cmake)

include(FetchContent)
FetchContent_Declare(
  Kokkos
  URL https://github.com/kokkos/kokkos/archive/refs/tags/4.5.01.zip
)
FetchContent_MakeAvailable(Kokkos)

add_custom_command(
    OUTPUT  ${CMAKE_SOURCE_DIR}/backend/libcharmtyles.def.h
    COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_SOURCE_DIR}/backend
            ${CHARMC} ${CMAKE_SOURCE_DIR}/backend/charmtyles.ci
    DEPENDS ${CMAKE_SOURCE_DIR}/backend/charmtyles.ci
    COMMENT "Processing ci files"
)

message(STATUS "CHARMC = ${CHARMC}")

set(CMAKE_CXX_COMPILER "${CHARMC}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OPTS} ${LD_OPTS}")

add_library(charmtyles STATIC ${CMAKE_SOURCE_DIR}/charmtyles.hpp ${CMAKE_SOURCE_DIR}/backend/libcharmtyles.def.h)
target_include_directories(charmtyles PRIVATE ${BASE_DIR})
target_link_libraries(charmtyles Kokkos::kokkos)

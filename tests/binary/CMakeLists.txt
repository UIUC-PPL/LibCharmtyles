cmake_minimum_required(VERSION 3.16)
project(charmTyles)

include(${CMAKE_SOURCE_DIR}/../../config.cmake)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(BUILD_SHARED_LIBS ON)

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

add_custom_command(
    OUTPUT  ${CMAKE_SOURCE_DIR}/base.decl.h
    COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_SOURCE_DIR}
            ${CHARMC} ${CMAKE_SOURCE_DIR}/base.ci
    DEPENDS ${CMAKE_SOURCE_DIR}/base.ci
    COMMENT "Processing base ci files"
)

set(DESTINATION_FILE "${CMAKE_BINARY_DIR}/_deps/kokkos-src/tpls/desul/include/desul/atomics/Config.hpp")
add_custom_command(
    OUTPUT  ${DESTINATION_FILE}
    COMMAND ${CMAKE_SOURCE_DIR}/../../patch.sh ${CMAKE_BINARY_DIR}
    COMMENT "Generating ${DESTINATION_FILE}"
)

add_executable(binary_include.out binary_include.cpp ${BASE_DIR}/charmtyles/backend/libcharmtyles.decl.h ${CMAKE_SOURCE_DIR}/base.decl.h ${DESTINATION_FILE})
target_include_directories(binary_include.out PRIVATE ${BASE_DIR} ${BASE_DIR}/charmtyles/backend ${EIGEN_DIR})
target_link_libraries(binary_include.out Kokkos::kokkos)

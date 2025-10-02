cmake_minimum_required(VERSION 3.16)

project(adore)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# set(CMAKE_POSITION_INDEPENDENT_CODE ON)
# set(BUILD_SHARED_LIBS ON)

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -O3 -march=native")
# include(FetchContent)

# FetchContent_Declare(
#   Kokkos
#   URL https://github.com/kokkos/kokkos/archive/refs/tags/4.5.01.zip
# )

# FetchContent_MakeAvailable(Kokkos)

set(GPU_JOKE "hahahahah_brooooooskoooooo")

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGPU_JOKE=\"${GPU_JOKE}\"")

# add_definitions(-DGPU_JOKE=\"${GPU_JOKE}\")

add_executable(pla pla.cc)
target_compile_definitions(pla PRIVATE GPU_JOKE="${GPU_JOKE}")
# target_link_libraries(pla Kokkos::kokkos)

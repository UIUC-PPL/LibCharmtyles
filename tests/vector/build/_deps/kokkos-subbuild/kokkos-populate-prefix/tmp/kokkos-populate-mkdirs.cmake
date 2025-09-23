# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/shogo/master/Kale/charmTylesUpstream/LibCharmtyles/tests/vector/build/_deps/kokkos-src"
  "/home/shogo/master/Kale/charmTylesUpstream/LibCharmtyles/tests/vector/build/_deps/kokkos-build"
  "/home/shogo/master/Kale/charmTylesUpstream/LibCharmtyles/tests/vector/build/_deps/kokkos-subbuild/kokkos-populate-prefix"
  "/home/shogo/master/Kale/charmTylesUpstream/LibCharmtyles/tests/vector/build/_deps/kokkos-subbuild/kokkos-populate-prefix/tmp"
  "/home/shogo/master/Kale/charmTylesUpstream/LibCharmtyles/tests/vector/build/_deps/kokkos-subbuild/kokkos-populate-prefix/src/kokkos-populate-stamp"
  "/home/shogo/master/Kale/charmTylesUpstream/LibCharmtyles/tests/vector/build/_deps/kokkos-subbuild/kokkos-populate-prefix/src"
  "/home/shogo/master/Kale/charmTylesUpstream/LibCharmtyles/tests/vector/build/_deps/kokkos-subbuild/kokkos-populate-prefix/src/kokkos-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/shogo/master/Kale/charmTylesUpstream/LibCharmtyles/tests/vector/build/_deps/kokkos-subbuild/kokkos-populate-prefix/src/kokkos-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/shogo/master/Kale/charmTylesUpstream/LibCharmtyles/tests/vector/build/_deps/kokkos-subbuild/kokkos-populate-prefix/src/kokkos-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()

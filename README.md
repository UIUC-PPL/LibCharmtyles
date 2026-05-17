# LibCharmtyles
C++ Backend to Python Charmtyles

Currently the standalone C++ support has been removed, and it's only supposed to be run with charmnumerics.
Settign it up needs just the Kokkos and Kokkos-kernels setup and the rest of the build systems is in charmnumerics
For building Kokkos and Kokkos kernels a script is provided [here](setup_gpu.sh)
You also might need to build eigen3 if it's not already download.
```
Note: We are working on resolving this dependency
```
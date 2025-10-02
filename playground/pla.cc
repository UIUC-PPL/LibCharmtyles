#include <iostream>
#include <dlfcn.h>
#include <fstream>
// #include <Kokkos_Core.hpp>
// #include <Kokkos_Macros.hpp>
// #include <iostream>
#include <string>

// double not_here(double x) {
//     return 42.0 + x;
// }

int main() {
    std::cout << std::string(GPU_JOKE) << std::endl;
//     std::string program = R"(
// #include <Kokkos_Core.hpp>

// struct MyKokkosFunctor {
//     Kokkos::View<double*> data_view;
//     double some_constant;
//     void* not_here_ptr;

//     KOKKOS_INLINE_FUNCTION MyKokkosFunctor(Kokkos::View<double*> data, double constant_val, void* not_here)
//         : data_view(data), some_constant(constant_val), not_here_ptr(not_here) {}

//     KOKKOS_INLINE_FUNCTION
//     void operator()(const int i) const {
//         data_view(i) = some_constant * i;
//         data_view(i) = ((double(*)(double))not_here_ptr)(data_view(i));
//     }
// };

// extern "C" void run_kernel(Kokkos::View<double*> data_view, double constant_val, void* not_here_ptr) {
//     MyKokkosFunctor my_functor(data_view, constant_val, not_here_ptr);
//     Kokkos::parallel_for("MyKernelLabel", Kokkos::RangePolicy<>(0, 100), my_functor);
// }
// )";
//     std::fstream ofs("test.cc", std::ios::out);
//     if (!ofs.is_open()) {
//         std::cerr << "Cannot open file: test.cc" << '\n';
//         return 1;
//     }
//     ofs << program;
//     ofs.close();
//     system("g++ -std=c++20 -I$PWD/_deps/kokkos-src/tpls/mdspan/include -I$PWD/_deps/kokkos-src/core/src -I$PWD/_deps/kokkos-build -shared -fPIC -o libtest.so test.cc -L$PWD/_deps/kokkos-build/core/src -lkokkoscore");

//     void* handle  = dlopen("./libtest.so", RTLD_NOW);
//     if(!handle) {
//         std::cerr << "Cannot open library: " << dlerror() << '\n';
//         return 1;
//     } else {
//         std::cout << "Library loaded successfully" << std::endl;
//     }
//     void* functor = dlsym(handle, "run_kernel");
//     if(!functor) {
//         std::cerr << "Cannot load symbol 'kernel': " << dlerror() << '\n';
//         dlclose(handle);
//         return 1;
//     } else {
//         std::cout << "Symbol loaded successfully" << std::endl;
//     }
//     Kokkos::initialize(); {
//         Kokkos::View<double*> data_view("data_view", 100);
//         ((void(*)(Kokkos::View<double*>, double, void*))functor)(data_view, 6, (void*)&not_here);
//         auto host_view = Kokkos::create_mirror_view(data_view);
//         Kokkos::deep_copy(host_view, data_view);
//         for(int i = 0; i < 10; ++i) {
//             std::cout << host_view(i) << " ";
//         }
//         std::cout << std::endl;
//     } Kokkos::finalize();

//     dlclose(handle);
//     system("rm test.cc libtest.so");

    return 0;
}

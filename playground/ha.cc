#include <iostream>
#include <cstdint>
#include <vector>

class base {
public:
    virtual void g() {
        std::cout << "base::g()" << std::endl;
    }
    virtual void fun() {
        std::cout << "base::fun()" << std::endl;
    }
    virtual void fun(int x) {
        std::cout << "base::fun(int)" << std::endl;
    }
};

class derived : public base {
public:
    virtual void fun() final {
        std::cout << "derived::fun()" << std::endl;
    }
    virtual void fun(int x) {
        std::cout << "derived::fun(int)" << std::endl;
    }
    virtual void g() {
        std::cout << "derived::g()" << std::endl;
    }
};

class derived_fake : public base {
public:
    virtual void fun() final {
        std::cout << "derived_fake::fun()" << std::endl;
    }
    virtual void fun(int x) {
        std::cout << "derived_fake::fun(int)" << std::endl;
    }
    virtual void g() {
        std::cout << "derived_fake::g()" << std::endl;
    }
};

void* ret_fn_ptr(base* b) {
    void** vtable = *reinterpret_cast<void***>(b);
    void* fun_ptr = vtable[1];
    using RawFun = void(*)(void);
    RawFun rf = reinterpret_cast<RawFun>(fun_ptr);
    return reinterpret_cast<void*>(rf);
}

#include <tuple>

class love {
public:
    using res_type = std::tuple<int&, double>;
};

int main() {
    std::vector<int> vec = {1, 2, 3, 4};

    // Move vec into a tuple
    std::tuple<std::vector<int>> t = std::make_tuple(std::move(vec));

    // vec is now empty
    std::cout << "vec.size() = " << vec.size() << "\n";
    std::cout << "tuple vector size = " << std::get<0>(t).size() << "\n";
    // derived d;
    // derived_fake df;
    // std::vector<void*> v;

    // base* b = &d;
    // v.push_back(ret_fn_ptr(b));

    // b = &df;
    // v.push_back(ret_fn_ptr(b));

    // for(auto it : v) {
    //     using Fun = void(*)(void);
    //     Fun f = reinterpret_cast<Fun>(it);
    //     f();
    // }

    return 0;
}


/*
    std::vector<Kokkos::View<double*>> kkVecViews;
    std::map<int, int> vecToKkVecMap;
    std::size_t kkVecViewIdx = 0;
*/
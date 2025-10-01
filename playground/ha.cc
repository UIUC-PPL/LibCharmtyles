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

void grow() {
    std::cout << "GROWWWWWW" << std::endl;
}

int main() {
    uintptr_t ka = (uintptr_t)&grow;
    ((void(*)(void))(void*)ka)();
    // std::tuple<int, std::string> t{
    //     1, std::string("hello")
    // };

    // uintptr_t ha = 78;

    // std::get<0>(t) = 42;

    // // for(auto it : std::get<0>(t)) {
    // std::cout << std::get<0>(t) << std::endl;
    // }

    // // Move the whole tuple
    // auto t2 = std::move(t); // moves vector and string into t2

    // std::cout << "t2 vector size: " << std::get<0>(t2).size() << "\n";
    // std::cout << "t vector size (moved-from): " << std::get<0>(t).size() << "\n";

    // size_t ha = -1;

    // if(ha != -1) {
    //     std::cout << "FUCKED" << std::endl;
    // }

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
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
    derived(int x) : a(x) {}
    virtual void fun() final {
        std::cout << "derived::fun()" << std::endl;
    }
    virtual void fun(int x) {
        std::cout << "derived::fun(int)" << std::endl;
        std::cout << a + x << std::endl;
    }
    virtual void g() {
        std::cout << "derived::g()" << std::endl;
    }
    int a;
};

class derived_fake : public base {
public:
    virtual void fun() final {
        std::cout << "derived_fake::fun()" << std::endl;
    }
    virtual void fun(int x) {
        std::cout << "derived_fake::fun(int)" << std::endl;
        std::cout << x << std::endl;
    }
    virtual void g() {
        std::cout << "derived_fake::g()" << std::endl;
    }
};

void* ret_fn_ptr(base* b) {
    void** vtable = *reinterpret_cast<void***>(b);
    void* fun_ptr = vtable[2];
    using RawFun = void(*)(void*, int);
    RawFun rf = reinterpret_cast<RawFun>(fun_ptr);
    return reinterpret_cast<void*>(rf);
}

int main() {
    derived d(42);
    derived_fake df;
    std::vector<void*> v;

    base* b = &d;
    v.push_back(ret_fn_ptr(b));

    b = &df;
    v.push_back(ret_fn_ptr(b));

    for(auto it : v) {
        using Fun = void(*)(void*, int);
        Fun f = reinterpret_cast<Fun>(it);
        f((void*)b, 27);
    }
    return 0;
}

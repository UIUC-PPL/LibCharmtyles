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

template<typename T>
void f(T&& x) {
    x = 27;
}



int main() {
    int x = 42;
    const int y = x;
    const int& z = x;

    f(x);

    f(y);

    f(z);

    f(69);
}

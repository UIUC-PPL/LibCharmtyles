#include <iostream>
#include <cstdint>
#include <vector>
#include <memory>

class base {
public:
    int a = 12;
    base() {
        std::cout << "base::base()" << std::endl;
    }
    virtual ~base() {
        std::cout << "base::~base()" << std::endl;
    }
    void garbage() const {
        std::cout << "base::garbage() const" << std::endl;
    }
    virtual void g() {
        std::cout << "base::g()" << std::endl;
    }
    void garbage() {
        std::cout << "base::garbage()" << std::endl;
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
    int d = 34;
    derived() {
        std::cout << "derived::derived()" << std::endl;
    }
    ~derived() {
        std::cout << "derived::~derived()" << std::endl;
    }
    void ahhahahha() const {
        std::cout << "derived::ahhahahha() const" << std::endl;
    }
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
    int e = 89;
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
    void* fun_ptr = vtable[3];
    using RawFun = void(*)(void);
    RawFun rf = reinterpret_cast<RawFun>(fun_ptr);
    return reinterpret_cast<void*>(rf);
}

int main() {
    // derived d;
    // derived_fake df;
    std::vector<void*> v;

    std::shared_ptr<base> b = std::make_shared<derived>();

    // base* b = &d;
    v.push_back(ret_fn_ptr(b.get()));

    // b = &df;
    // v.push_back(ret_fn_ptr(b));

    for(auto it : v) {
        using Fun = void(*)(void);
        Fun f = reinterpret_cast<Fun>(it);
        f();
    }

    return 0;
}

#include <iostream>
#include <type_traits>

template<typename a_v, typename b_v>
class ano {
public:
    a_v a;
    b_v b;
};

class fake {
    template<typename LHS, typename RHS>
    friend class ano;
};

class broosko {

};

template<typename T>
struct shogo_type {
    constexpr static bool val = false;
};

template<typename... Ts>
struct shogo_type<ano<Ts...>> {
    constexpr static bool val = true;
};

template<>
struct shogo_type<broosko> {
    constexpr static bool val = false;
};

template<typename A>
A foo(A a) {
    return a + a;
}

template<>
std::string foo(std::string a) {
    return "ho";
}

template<typename T>
bool check(T a) {
    return shogo_type<typename std::decay<T>::type>::val;
}

int main() {
    // std::string uo = "ha";
    // std::cout << foo(uo) << std::endl;
    fake faker;
    // ano<int, bool> chan;

    if(check(faker)) {
        printf("HUZZZAH\n");
    }
}

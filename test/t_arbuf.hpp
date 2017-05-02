#include <cpptest.hpp>
#include "../array_buffer.hpp"

namespace t_arbuf {
    
bool current() {
    int a = 0, b = 0;
    CASE::ArrayBuffer<int> arb{&a, &b};
    return arb.current() == &a;
}

bool next() {
    int a = 0, b = 0;
    CASE::ArrayBuffer<int> arb{&a, &b};
    return arb.next() == &b;
}

bool flip() {
    int a = 0, b = 0;
    CASE::ArrayBuffer<int> arb{&a, &b};
    arb.flip();
    return arb.current() == &b && arb.next() == &a;
}

bool access() {
    int xs[10] = { 0,1,2,3,4,5,6,7,8,9 };
    CASE::ArrayBuffer<int> arb{xs, xs + 5};
    const auto t1 = arb[2] == 2;
    arb.flip();
    return t1 && arb[2] == 7;
}

void run() {
    cpptest::Module test{"array buffer"};
    test.fn("current", current);
    test.fn("next", next);
    test.fn("access", access);
}

} // t_arbuf

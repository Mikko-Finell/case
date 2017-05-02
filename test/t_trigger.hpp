#include <cpptest.hpp>
#include "../trigger.hpp"

namespace t_trigger {

bool trigger01() {
    CASE::Trigger tr{5.0, .20};
    return
    (tr.update(3) == false) &&
    (tr.update(4) == false) &&
    (tr.update(5) == false) &&
    (tr.update(6) == false) &&
    (tr.update(7) == true) &&
    (tr.update(8) == true) &&
    (tr.update(9) == true) &&
    (tr.update(8) == true) &&
    (tr.update(7) == true) &&
    (tr.update(6) == true) &&
    (tr.update(5) == true) &&
    (tr.update(4) == true) &&
    (tr.update(3) == false) &&
    (tr.update(2) == false) &&
    (tr.update(7) == true) &&
    (tr.update(3) == false) &&
    (tr.update(8) == true) &&
    (tr.update(1) == false) &&
    (tr.update(9) == true);
}

bool trigger02() {
    const auto SIZE = 10;
    CASE::Trigger tr{25, .2, SIZE};
    for (int i = 0; i < SIZE; i++)
        tr.update(31);
    const auto t0 = tr.read_state() == true;

    for (int i = 0; i < SIZE; i++)
        tr.update(21);
    const auto t1 = tr.read_state() == true;

    for (int i = 0; i < SIZE; i++)
        tr.update(19);
    const auto t2 = tr.read_state() == false;

    for (int i = 0; i < SIZE; i++)
        tr.update(29);
    const auto t3 = tr.read_state() == false;

    return t0 && t1 && t2 && t3;
}

void run() {
    cpptest::Module test{"trigger"};
    test.fn("01", trigger01);
    test.fn("02", trigger02);
}

} // t_trigger

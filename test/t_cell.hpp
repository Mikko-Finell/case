#include <cpptest.hpp>
#include "../cell.hpp"

namespace t_cell {

template<int STACKSIZE>
class t1Agent {
    bool status = true;

public:
    CASE::ZCell<t1Agent, STACKSIZE> * cell = nullptr;
    int z = 0;
    t1Agent(int depth = 0) : z(depth) {}

    bool active() { return status; }
    void activate() { status = true; }
    void deactivate() { status = false; }
};

class t2Agent {
public:
    CASE::ZCell<t2Agent, 2> * cell = nullptr;
    int z = 0;
    t2Agent(int depth = 0) : z(depth) {}

    bool status = true;
    bool active() const { return status; }
};

bool insert() {
    CASE::ZCell<t1Agent<2>, 2> cell;
    t1Agent<2> a, b{1}, c;
    return cell.insert(a) == CASE::Code::OK
        && cell.insert(b) == CASE::Code::OK
        && cell.insert(c) == CASE::Code::Rejected;
}

bool extract() {
    CASE::ZCell<t1Agent<3>, 3> cell;
    t1Agent<3> a{0}, b{1}, c{2};
    cell.insert(a);
    cell.insert(b);
    cell.insert(c);
    return cell.extract(a.z) == &a && cell.popcount() == 2
        && cell.extract(b.z) == &b && cell.popcount() == 1
        && cell.extract(c.z) == &c && cell.is_empty()
        && cell.extract_top() == nullptr;
}

bool get() {
    CASE::ZCell<t1Agent<3>, 3> cell;
    t1Agent<3> a, b, c;
    cell.insert(a);
    cell.insert(b);
    cell.insert(c);
    return cell.get() == &a;
}

bool clear() {
    CASE::ZCell<t1Agent<3>, 3> cell;
    t1Agent<3> a, b, c;
    cell.insert(a);
    cell.insert(b);
    cell.insert(c);
    cell.clear();
    return a.cell == nullptr && b.cell == nullptr && c.cell == nullptr
        && cell.is_empty();
}

bool insertion0() {
    CASE::SimpleCell<t1Agent<1>> cell0, cell1;
    t1Agent<1> a, b;
    cell0.insert(a);
    cell1.insert(b);
    return a.cell == &cell0 && b.cell == &cell1;
}

bool relocation0() {
    CASE::ZCell<t1Agent<2>, 2> cell0, cell1;
    t1Agent<2> a, b{1};

    cell0.insert(a);
    cell1.insert(b);

    cell1.insert(cell0.extract_top());

    return b.cell == &cell1 && a.cell == &cell1;
}

bool extraction0() {
    CASE::SimpleCell<t1Agent<1>> cell;
    t1Agent<1> a;
    cell.insert(a);
    assert(a.cell == &cell);
    cell.extract(a.z);
    return a.cell == nullptr;
}

bool insertion1() {
    CASE::ZCell<t2Agent, 2> cell;
    t2Agent a;
    assert(cell.insert(a) == CASE::Code::OK);
    assert(cell.is_occupied());
    a.status = false;
    return cell.is_empty();
}

bool relocation1() {
    CASE::ZCell<t2Agent, 2> cell0, cell1;
    t2Agent a;
    cell0.insert(a);
    return cell1.insert(a) == CASE::Code::Rejected;
}

bool extraction1() {
    CASE::ZCell<t2Agent, 2> cell;
    t2Agent a;
    assert(cell.insert(a) == CASE::Code::OK);
    assert(cell.getlayer(0) == &a);
    a.status = false;
    return cell.extract(0) == nullptr;
}

void run() {
    cpptest::Module test0{"ZCell"};
    test0.fn("insert", insert);
    test0.fn("extract", extract);
    test0.fn("get", get);
    test0.fn("clear", clear);

    cpptest::Module test1{"agent assigned cell"};
    test1.fn("insertion", insertion0);
    test1.fn("relocation", relocation0);
    test1.fn("extraction", extraction0);

    cpptest::Module test2{"deactivated agent removed"};
    test2.fn("insertion", insertion1);
    test2.fn("relocation", relocation1);
    test2.fn("extraction", extraction1);
}
}

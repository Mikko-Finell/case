#include <cpptest.hpp>
#include "../cell.hpp"

namespace t_cell {

template<int STACKSIZE>
class t1Agent {
    bool status = true;

public:
    CASE::ZCell<t1Agent, STACKSIZE> * cell = nullptr;
    bool active() { return status; }
    void activate() { status = true; }
    void deactivate() { status = false; }
};

class t2Agent {
public:
    CASE::ZCell<t2Agent, 2> * cell = nullptr;
    bool status = true;
    bool active() const { return status; }
};

bool insert() {
    CASE::ZCell<t1Agent<2>, 2> cell;
    t1Agent<2> a, b, c;
    return cell.insert(a, 0) == CASE::Code::OK
        && cell.insert(b, 1) == CASE::Code::OK
        && cell.insert(c) == CASE::Code::Rejected;
}

bool extract() {
    CASE::ZCell<t1Agent<3>, 3> cell;
    t1Agent<3> a, b, c;
    cell.insert(a, 0);
    cell.insert(b, 1);
    cell.insert(c, 2);
    return cell.extract() == &a && cell.popcount() == 2
        && cell.extract() == &b && cell.popcount() == 1
        && cell.extract() == &c && cell.is_empty()
        && cell.extract() == nullptr;
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
    t1Agent<2> a, b;

    cell0.insert(a, 0);
    cell1.insert(b, 0);
    cell1.insert(cell0.extract(), 1);

    bool t0 = b.cell == &cell1;
    return b.cell == &cell1 && a.cell == &cell1;
}

bool extraction0() {
    CASE::SimpleCell<t1Agent<1>> cell;
    t1Agent<1> a;
    cell.insert(a);
    assert(a.cell == &cell);
    cell.extract();
    return a.cell == nullptr;
}

bool insertion1() {
    CASE::ZCell<t2Agent, 2> cell;
    t2Agent a;
    assert(cell.insert(a, 0) == CASE::Code::OK);
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
    assert(cell.insert(a, 0) == CASE::Code::OK);
    assert(cell.get(0) == &a);
    a.status = false;
    return cell.extract( 0 ) == nullptr;
}

bool add_to_layer() {
    return false;
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
    
    cpptest::Module bug01{"test for bug on 17.5.2017"};
    bug01.fn("add agent to sublayer", add_to_layer);
}
}

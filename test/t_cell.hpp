#include <cpptest.hpp>
#include "../cell.hpp"
#include "../world.hpp"

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
    void activate() { status = true; }
    void deactivate() { status = false; }
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
        && cell.extract(0) == nullptr;
}

bool get() {
    CASE::ZCell<t1Agent<3>, 3> cell;
    t1Agent<3> a, b, c;
    cell.insert(a);
    cell.insert(b);
    cell.insert(c);
    return cell.getlayer(0) == &a;
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
    CASE::ZCell<t1Agent<1>, 1> cell0, cell1;
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

    cell1.insert(cell0.extract(0));

    return b.cell == &cell1 && a.cell == &cell1;
}

bool extraction0() {
    CASE::ZCell<t1Agent<1>, 1> cell;
    t1Agent<1> a;
    cell.insert(a);
    assert(a.cell == &cell);
    cell.extract(a.z);
    return a.cell == nullptr && cell.getlayer(a.z) == nullptr;
}

bool reject() {
    CASE::ZCell<t2Agent, 2> cell;
    t2Agent a, b;
    a.activate();
    b.activate();

    assert(cell.insert(a) == CASE::OK);

    bool test = cell.insert(b) == CASE::Rejected;

    return test;
}

bool insertion1() {
    CASE::ZCell<t2Agent, 2> cell_a, cell_b;
    t2Agent a;
    cell_a.insert(a);

    assert(cell_a.getlayer(a.z) == &a);
    
    cell_b.insert(a);

    return a.cell == &cell_b && cell_b.getlayer(a.z) == &a
        && cell_a.getlayer(a.z) == nullptr;
}

class t3Agent {
    bool alive = false;
    bool updated = false;

    using Cell = CASE::ZCell<t3Agent, 2>;

public:
    int z = 0;
    Cell * cell = nullptr;

    void update() { updated = true; }
    bool was_updated() const { return updated; }
    void activate() { alive = true; }
    void deactivate() { alive = false; }
    bool active() const { return alive; }
};

} // t_cell

namespace t_cell {

bool spawn() {
    using namespace CASE;

    World<t3Agent> world{3};
    ZCell<t3Agent, 2> cell{world};

    auto ptr = cell.spawn(new t3Agent);

    return ptr != nullptr && ptr->active() == true;
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
    test1.fn("full cell insert rejected", reject);

    cpptest::Module test2{"old cell agent removed"};
    test2.fn("insertion", insertion1);

    cpptest::Module test3{"spawning"};
    test3.fn("spawn", spawn);
}
}

#include <cpptest.hpp>
#include "../cell.hpp"

namespace t_cell {

template<int STACKSIZE>
class Agent {
public:
    CASE::StackCell<Agent, STACKSIZE> * cell;
};

bool insert() {
    CASE::StackCell<Agent<2>, 2> cell;
    Agent<2> a, b, c;
    return cell.insert(a) == CASE::Code::OK
        && cell.insert(b) == CASE::Code::OK
        && cell.insert(c) == CASE::Code::Rejected;
}

bool extract() {
    CASE::StackCell<Agent<3>, 3> cell;
    Agent<3> a, b, c;
    cell.insert(a, b, c);
    return cell.extract() == &c && cell.popcount() == 2
        && cell.extract() == &b && cell.popcount() == 1
        && cell.extract() == &a && cell.is_empty()
        && cell.extract() == nullptr;
}

bool get() {
    CASE::StackCell<Agent<3>, 3> cell;
    Agent<3> a, b, c;
    cell.insert(a);
    cell.insert(b);
    cell.insert(c);
    return cell.get() == &c;
}

bool clear() {
    CASE::StackCell<Agent<3>, 3> cell;
    Agent<3> a, b, c;
    cell.insert(a, b, c);
    cell.clear();
    return a.cell == nullptr && b.cell == nullptr && c.cell == nullptr
        && cell.is_empty();
}

bool insertion() {
    CASE::SimpleCell<Agent<1>> cell0, cell1;
    Agent<1> a, b;
    cell0.insert(a);
    cell1.insert(b);
    return a.cell == &cell0 && b.cell == &cell1;
}

bool relocation() {
    CASE::StackCell<Agent<2>, 2> cell0, cell1;
    Agent<2> a, b;
    cell0.insert(a, b);
    cell1.insert(cell0.extract());
    bool t0 = b.cell == &cell1;
    return t0 && a.cell == &cell0;
}

bool extraction() {
    CASE::SimpleCell<Agent<1>> cell;
    Agent<1> a;
    cell.insert(a);
    assert(a.cell == &cell);
    cell.extract();
    return a.cell == nullptr;
}

void run() {
    cpptest::Module test0{"stackcell"};
    test0.fn("insert", insert);
    test0.fn("extract", extract);
    test0.fn("get", get);
    test0.fn("clear", clear);

    cpptest::Module test1{"agent assigned cell"};
    test1.fn("insertion", insertion);
    test1.fn("relocation", relocation);
    test1.fn("extraction", extraction);
}
}

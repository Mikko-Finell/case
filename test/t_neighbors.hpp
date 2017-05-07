#include <cpptest.hpp>
#include "../neighbors.hpp"

namespace t_neighbors {

class Cell {
    int value = 0;
public:
    void set(const int x) { value = x; }
    int get() const { return value; }
};

bool set() {
    CASE::Neighbors<Cell> nh;
    
    Cell cells[9];
    int n = 0;
    for (int y : {-1, 0, 1}) {
        for (int x : {-1, 0, 1})
            nh.set(x, y, cells[n++]);
    }

    return &nh(-1, -1) == &cells[0]
        && &nh( 0, -1) == &cells[1]
        && &nh( 1, -1) == &cells[2]
        && &nh(-1,  0) == &cells[3]
        && &nh( 0,  0) == &cells[4]
        && &nh( 1,  0) == &cells[5]
        && &nh(-1,  1) == &cells[6]
        && &nh( 0,  1) == &cells[7]
        && &nh( 1,  1) == &cells[8];
}

bool manipulate() {
    CASE::Neighbors<Cell> nh;
    
    Cell cells[9];
    int n = 0;
    for (int y : {-1, 0, 1}) {
        for (int x : {-1, 0, 1})
            nh.set(x, y, cells[n++]);
    }

    nh(1, 1).set(123);
    nh(-1, -1).set(321);
    return cells[8].get() == 123 && cells[0].get() == 321;
}

bool const_check(const CASE::Neighbors<Cell> & nh, const int value) {
    return nh(1, 0).get() == 444;
}

bool qconst() {
    CASE::Neighbors<Cell> nh;
    
    Cell cells[9];
    int n = 0;
    for (int y : {-1, 0, 1}) {
        for (int x : {-1, 0, 1})
            nh.set(x, y, cells[n++]);
    }

    nh(1, 0).set(444);
    return const_check(nh, 444);
}

void run() {
    cpptest::Module test{"neighbors"};
    test.fn("set", set);
    test.fn("manipulate", manipulate);
    test.fn("query const", qconst);
}
}

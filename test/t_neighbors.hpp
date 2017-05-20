#include <algorithm>
#include <vector>
#include <cpptest.hpp>
#include "../neighbors.hpp"
#include "../cell.hpp"

namespace t_neighbors {

class Agent {
public:
    int z = 0;
    CASE::SimpleCell<Agent> * cell = nullptr;
    bool operator==(const Agent & other) const { return &other == this; }
    bool operator!=(const Agent & other) const { return !(other == *this); }
    bool active() { return true; }
};

using Cell = CASE::SimpleCell<Agent>;

template<class T>
void init(T * cells, CASE::Neighbors<T> & nh) {
    static const int range[3] = {-1, 0, 1};
    for (const auto y : range) {
        for (const auto x : range)
            nh.assign_cell(*(cells++), x, y);
    }
}

bool assign() {
    CASE::Neighbors<Cell> nh;
    Cell cells[9];
    init(cells, nh);
    Agent agents[9];
    for (int i = 0; i < 9; i++)
        cells[i].insert(agents[i]);
    std::vector<bool> results;
    static const int range[3] = {-1, 0, 1};
    auto i = 0;
    for (const auto y : range) {
        for (const auto x : range)
            results.push_back(nh(x, y) == cells[i++].get());
    }
    return std::all_of(results.begin(), results.end(), [](bool b){return b;});
}

bool insert() {
    CASE::Neighbors<Cell> nh;
    Cell cells[9];
    init(cells, nh);
    Agent agents[9];
    auto i = 0;
    static const int range[3] = {-1, 0, 1};
    for (const auto y : range) {
        for (const auto x : range) {
            nh.insert(agents[i++]).at(x, y);
        }
    }
    std::vector<bool> results;
    i = 0;
    for (const auto y : range) {
        for (const auto x : range) {
            assert(nh(x, y) != nullptr);
            results.push_back(nh(x, y) == &agents[i++]);
        }
    }

    return std::all_of(results.begin(), results.end(), [](bool b){return b;});
}

bool transplant() {
    CASE::Neighbors<Cell> nh;
    Cell cells[9];
    init(cells, nh);
    Agent a;
    assert(nh.insert(a, -1, -1) == CASE::Code::OK);

    nh.transplant().from(-1, -1).to(1, 1);
    return nh(1, 1) == &a && nh.cell_is_empty(-1, -1);
}

bool popcount() {
    bool t0 = false, t1 = false, t2 = false;
    CASE::Neighbors<Cell> nh;
    Cell cells[9];
    init(cells, nh);
    Agent a, b, c, d;
    nh.insert(a, -1, -1);
    t0 = nh.popcount() == 1;
    nh.insert(b, 0, -1);
    nh.insert(c, 1, -1);
    t1 = nh.popcount() == 3;
    nh.insert(d, 1, 0);
    nh.extract(1, 0);
    t2 = nh.popcount() == 2;
    nh.insert(d).at(1, 0);
    return t0 && t1 && nh.popcount() == 4;
}

bool swap() {
    CASE::Neighbors<Cell> nh;
    Cell cells[9];
    init(cells, nh);
    Agent a, b;
    nh.insert(a).at(-1, -1);
    nh.insert(b).at(1, 1);
    nh.swap(-1, -1).with(1, 1);
    return nh(-1, -1) == &b && nh(1, 1) == &a;
}

bool clear() {
    CASE::Neighbors<Cell> nh;
    Cell cells[9];
    init(cells, nh);

    Agent agents[9];
    auto i = 0;
    const static int range[3] = {-1, 0, 1};
    for (const auto y : range) {
        for (const auto x : range)
            nh.insert(agents[i++], x, y);
    }
    nh.clear(1, 1);
    bool t0 = nh.popcount() == 8;
    nh.clear();
    return t0 && nh.popcount() == 0;
}

bool op0() {
    CASE::Neighbors<Cell> nh;
    Cell cells[9];
    init(cells, nh);
    Agent agents[4];

    nh.insert(agents[0], -1, -1);
    nh.insert(agents[1],  1, -1);
    nh.insert(agents[2],  1,  1);
    nh.insert(agents[3], -1,  1);

    auto list = nh();

    bool all_are_in = std::all_of(std::begin(agents), std::end(agents),
        [&](auto & agent)
        {
            return std::any_of(list.cbegin(), list.cend(),
                [&](auto ptr){ return ptr == &agent; });
        });

    return list.size() == 4 && all_are_in;
}

class zAgent {
public:
    int z = 0;
    CASE::ZCell<zAgent,2> * cell = nullptr;
    bool active() { return true; }
};

bool op1() {
    CASE::OnGrid<CASE::ZCell<zAgent, 2>> nh;
    CASE::ZCell<zAgent, 2> cells[9];
    init(cells, nh);
    zAgent agents[4];
    agents[1].z = 1;
    agents[2].z = 1;

    nh.insert(agents[0], -1, -1);
    nh.insert(agents[1],  1, -1);
    nh.insert(agents[2],  1,  1);
    nh.insert(agents[3], -1,  1);

    auto list = nh.gather<zAgent>([](const zAgent & agent){ return agent.z; });

    std::vector<bool> results;
    for (auto & agent : agents) {
        if (agent.z) 
            results.push_back(std::any_of(list.cbegin(), list.cend(),
                [&](auto ptr){ return ptr == &agent; }));
        else
            results.push_back(std::none_of(list.cbegin(), list.cend(),
                [&](auto ptr){ return ptr == &agent; }));
    }

    return list.size() == 2 && std::all_of(results.begin(), results.end(),
        [](bool b){return b;});
}

void run() {
    cpptest::Module test{"neighbors"};
    test.fn("assign", assign);
    test.fn("insert", insert);
    //test.fn("transplant", transplant);
    test.fn("popcount", popcount);
    test.fn("swap", swap);
    test.fn("clear", clear);
    test.fn("operator()", op0);
    test.fn("operator(predicate)", op1);
}
}

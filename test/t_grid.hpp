#include <future>
#include <string>
#include <algorithm>
#include <vector>
#include <cpptest.hpp>
#include "../array_buffer.hpp"
#include "../cell.hpp"
#include "../grid.hpp"
#include "../neighbors.hpp"
#include "../timer.hpp"

namespace t_grid {

bool wrap(int size, int index, int expected) {
    return CASE::wrap(index, size) == expected;
}

bool wrap_forward() {
    return wrap(4, 0, 0)
        && wrap(4, 1, 1)
        && wrap(4, 2, 2)
        && wrap(4, 3, 3)
        && wrap(4, 4, 0)
        && wrap(4, 5, 1)
        && wrap(4, 6, 2)
        && wrap(4, 7, 3)
        && wrap(4, 8, 0)
        && wrap(4, 9, 1)
        && wrap(4, 10, 2);
}

bool wrap_backward() {
    return wrap(4, 0, 0)
        && wrap(4, -1, 3)
        && wrap(4, -2, 2)
        && wrap(4, -3, 1)
        && wrap(4, -4, 0)
        && wrap(4, -5, 3)
        && wrap(4, -6, 2)
        && wrap(4, -7, 1)
        && wrap(4, -8, 0)
        && wrap(4, -9, 3)
        && wrap(4, -10, 2);
}

bool rm(int cols, int rows) {
    auto ptr = new int[cols * rows];
    for (auto i = 0; i < cols * rows; i++)
        ptr[i] = i;

    int i = 0;
    std::vector<bool> results;
    for (int y = 0; y < rows; y++) {
        for (int x = 0; x < cols; x++) {
            results.push_back(CASE::index(x, y, cols) == i);
            i++;
        }
    }
    delete [] ptr;
    return std::all_of(results.begin(), results.end(), [](bool b){ return b; });
}

struct Pos {
    int x=666, y=777;
    bool operator==(const Pos & other) const {
        return other.x == x && other.y == y;
    }
};

class Agent {
    bool alive = false;

public:
    int x, y;
    int z = 0;
    bool updated = false;

    CASE::ZCell<Agent, 1> * cell = nullptr;
    bool active() { return alive; }
    void activate() { alive = true; }
    void deactivate() { alive = false; }
    
    template <class Grid>
    void update(Grid&) {
        updated = true;
    }
};

using namespace CASE;

bool update() {
    Grid<ZCell<Agent, 1>> grid{2, 2};
    Agent model;

    int i = 0;
    for (int y = 0; y < 2; y++) {
        for (int x = 0; x < 2; x++)
            grid.spawn(model, x, y);
    }
    assert(grid.agent_count() == 4);
    grid.update(4);

    assert(grid.agent_count() == 4);

    /*
    for (int i = 0; i < 4; i++)
        std::cout << &grid.cells[i] << " agent = "
            << grid.cells[i].getlayer(0) << std::endl;
    for (int i = 0; i < 4; i++) 
        std::cout << &grid.agents[i] << " cell = "
            << grid.agents[i].cell << std::endl;
    */
    
    bool test[4] = {0,0,0,0};
    for (int i = 0; i < 4; i++)
        test[i] = grid.agents[i].updated;

    return test[0]&&test[1]&&test[2]&&test[3];
}

bool spawn0() {
    Grid<ZCell<Agent, 1>> grid{2, 2};
    Agent model;

    for (int y = 0; y < 2; y++)
        for (int x = 0; x < 2; x++)
            grid.spawn(model, x, y);

    grid.update(4);
    assert(grid.agent_count() == 4);

    for (int y = 0; y < 2; y++)
        for (int x = 0; x < 2; x++)
            grid(x,y).getlayer(0)->deactivate();

    grid.update(4);
    assert(grid.agent_count() == 0);

    grid.spawn(model, 0, 0);
    grid.spawn(model, 1, 0);
    grid.spawn(model, 0, 1);
    grid.spawn(model, 1, 1);

    grid.update(4);

    return(grid.agent_count() == 4);
}

bool spawn1() {
    Grid<ZCell<Agent, 1>> grid{2, 2};
    Agent model;

    for (int y = 0; y < 2; y++)
        for (int x = 0; x < 2; x++)
            grid.spawn(model, x, y);

    grid.update(4);
    assert(grid.agent_count() == 4);

    for (auto i = 0; i < 4; i++)
        grid.agents[i].updated = false;
    
    grid.agents[0].deactivate();
    grid.agents[1].deactivate();

    grid.update(4);
    bool test[4] = {0,0,0,0};
    for (auto i = 0; i < 4; i++) {
        if (grid.agents[i].active())
            test[i] = grid.agents[i].updated;
        else
            test[i] = grid.agents[i].updated == false;
    }
    return grid.agent_count() == 2 && test[0]&&test[1]&&test[2]&&test[3];
}

bool spawn2() {
    Grid<ZCell<Agent, 1>> grid{2, 2};
    Agent model;

    auto p0 = grid.spawn(model, 0, 0);
    auto p1 = grid.spawn(model, 1, 0);
    auto p2 = grid.spawn(model, 0, 1);
    auto p3 = grid.spawn(model, 1, 1);

    grid.update(4);
    assert(grid.agent_count() == 4);

    grid(1, 0).getlayer(0)->deactivate();
    grid(1, 1).getlayer(0)->deactivate();

    grid.update(4);

    auto p4 = grid.spawn(model, 1, 0);
    auto p5 = grid.spawn(model, 1, 1);

    for (int y = 0; y < 2; y++)
        for (int x = 0; x < 2; x++)
            assert(grid.spawn(model, x, y) == nullptr);

    return false;
}

bool changecell() {
    Grid<ZCell<Agent, 1>> grid{2, 2};
    Agent model;

    for (int y = 0; y < 2; y++)
        for (int x = 0; x < 2; x++)
            grid.spawn(model, x, y);

    grid.update(4);

    auto agent = grid(0, 0).getlayer(0);
    auto t0 = grid(1, 1).insert(agent) == CASE::Rejected;

    grid(1, 1).getlayer(0)->deactivate();;

    auto t1 = grid(1, 1).insert(agent) == CASE::OK;

    return t0 && t1 && agent->cell == & grid(1, 1);
}

bool tkill() {
    Grid<ZCell<Agent, 1>> grid{2, 2};
    Agent model;

    for (int y = 0; y < 2; y++)
        for (int x = 0; x < 2; x++)
            grid.spawn(model, x, y);

    grid.update(4);

    grid.kill(*grid(0,0).getlayer(0));
    grid.kill(*grid(1,0).getlayer(0));
    grid.kill(*grid(0,1).getlayer(0));
    grid.kill(*grid(1,1).getlayer(0));

    bool t[4]={0,0,0,0};

    for (auto i = 0; i < 4; i++)
        t[i] = grid.agents[i].active() == false;

    return grid.agent_count() == 0 && t[0]&&t[1]&&t[2]&&t[3];
}

void run() {
    cpptest::Module twrap{"wrap"};
    twrap.fn("wrap forwards", wrap_forward);
    twrap.fn("wrap backwards", wrap_backward);

    cpptest::Module tindex{"row major index"};
    for (auto x = 1; x < 100; x++) {
        const auto name = std::to_string(x) + "x1";
        tindex.fn(name, [x]{ return rm(x, 1); });
    }
    for (auto y = 1; y < 100; y++) {
        const auto name = std::string("1x") + std::to_string(y);
        tindex.fn(name, [y]{ return rm(1, y); });
    }
    for (auto k = 1; k < 100; k++) {
        const auto name = std::to_string(k) + "x" + std::to_string(k);
        tindex.fn(name, [k]{ return rm(k, k); });
    }

    cpptest::Module grid{"grid"};
    grid.fn("update", update);
    grid.fn("spawn 0", spawn0);
    grid.fn("die: not updated", spawn1);
    grid.fn("spawn in cell", spawn2);
    grid.fn("change cells", changecell);
    grid.fn("kill", tkill);
}

}

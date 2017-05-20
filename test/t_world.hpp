#include "../world.hpp"
#include "../grid.hpp"
#include "../cell.hpp"
#include <cpptest.hpp>

namespace t_world {
    
class Agent {
    bool live = false;
public:
    Agent(int _x=0, int _y=0, int _z=0) :x(_x),y(_y),z(_z){}
    CASE::ZCell<Agent, 2> * cell = nullptr;
    bool active() const { return live; }
    void activate() { live = true; }
    void deactivate() { live = false; }
    int x = 0, y = 0, z = 0;
};

bool try_spawn_1() {
    using namespace CASE;
    using _grid = Grid<ZCell<Agent, 2>>;
    const int cols = 3, rows = 3;
    const int size = cols * rows;

    World<Agent, _grid> world{size};
    world.grid = _grid{cols, rows};

    Agent agent;

    auto ptr = world.try_spawn(agent);
    const auto t0 = ptr != nullptr;
    const auto t1 = world.try_spawn(agent) == nullptr;

    return t0 && t1;
}

bool try_spawn_2() {
    using namespace CASE;
    using _grid = Grid<ZCell<Agent, 2>>;
    const int cols = 3, rows = 3;
    const int size = cols * rows;

    World<Agent, _grid> world{size};
    world.grid = _grid{cols, rows};

    const auto t0 = world.try_spawn(Agent{1,1,0}) != nullptr;
    const auto t1 = world.try_spawn(Agent{1,1,1}) != nullptr;
    const auto t2 = world.try_spawn(Agent{0,0,0}) != nullptr;
    const auto t3 = world.try_spawn(Agent{0,0,0}) == nullptr;

    return t0 && t1 && t2 && t3;
}

bool try_move_1() {
    using namespace CASE;
    using _grid = Grid<ZCell<Agent, 2>>;
    const int cols = 3, rows = 3;
    const int size = cols * rows;

    World<Agent, _grid> world{size};
    world.grid = _grid{cols, rows};

    auto a0 = world.try_spawn(Agent{1,1,0});
    auto a1 = world.try_spawn(Agent{1,1,1});
    auto a2 = world.try_spawn(Agent{0,0,0});

    const auto t0 = world.try_move(*a0, 0, 0) == Code::Rejected;
    const auto t0_a = a0->x == 1 && a0->y == 1;

    const auto t1 = world.try_move(*a1).to(0, 0) == Code::OK;
    const auto t1_a = a1->x == 0 & a1->y == 0;

    return t0 && t1 && t0_a && t1_a;
}

void run() {
    cpptest::Module test{"world"};
    test.fn("try spawn 1", try_spawn_1);
    test.fn("try spawn 2", try_spawn_2);
    test.fn("try move 1", try_move_1);
}

}

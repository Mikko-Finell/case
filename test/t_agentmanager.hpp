#include "../random.hpp"
#include "../agent_manager.hpp"
#include "../grid.hpp"
#include "../cell.hpp"
#include <cpptest.hpp>

namespace t_agentmanager {

using namespace CASE;

class Agent {
    bool alive = false;
    bool updated = false;

    using Cell = ZCell<Agent, 2>;

public:
    int z = 0;
    Cell * cell = nullptr;

    void update() { updated = true; }
    bool was_updated() const { return updated; }
    void activate() { alive = true; }
    void deactivate() { alive = false; }
    bool active() const { return alive; }
};

bool spawn_1() {
    AgentManager<Agent> world{1};
    auto ptr = world.spawn(new Agent);
    return ptr != nullptr && world.spawn(new Agent) == nullptr;
}

bool spawn_2() {
    AgentManager<Agent> world{2};
    
    auto agent1 = new Agent, agent2 = new Agent;

    agent1 = world.spawn(agent1);
    agent2 = world.spawn(agent2);
    
    auto agent3 = world.spawn(new Agent);
    
    return agent1 != nullptr && agent2 != nullptr && agent3 == nullptr;
}

bool spawn_3() {
    AgentManager<Agent> world{2};
    
    auto agent1 = new Agent, agent2 = new Agent;

    agent1 = world.spawn(agent1);
    agent2 = world.spawn(agent2);
    
    agent1->deactivate();

    return world.spawn(new Agent) != nullptr;
}

bool cellspawn() {
    AgentManager<Agent> man{2};

    ZCell<Agent, 2> cell;
    cell.set_manager(man);

    auto agent1 = cell.spawn(new Agent);
    const auto t1 = agent1 != nullptr;
    const auto t2 = cell.spawn(new Agent) == nullptr;
    agent1->deactivate();
    auto agent2 = cell.spawn(new Agent);
    const auto t3 = agent2 != nullptr;

    return t1 && t2 && t3 && cell.spawn(new Agent) == nullptr;
}

bool updateextract() {
    Random rng;
    const auto popsize = rng(1, 512);
    AgentManager<Agent> man{popsize};
    auto cells = new ZCell<Agent, 2>[popsize];
    for (auto i = 0; i < popsize; i++)
        cells[i].set_manager(man);
    for (auto i = 0; i < popsize; i++)
        man.spawn(new Agent);
    
    assert(man.popcount() == popsize);
    for (auto i = 0; i < popsize; i++) {
        if (cells[i][0] != nullptr && rng.boolean())
            cells[i][0]->deactivate();
    }
    man.update();

    bool test = true;
    for (auto i = 0; i < popsize; i++) {
        auto agent = cells[i].getlayer(0);
        if (agent != nullptr)
            test = test && agent->active() == true;
    }

    delete [] cells;
    return test;
}
    
void run() {
    cpptest::Module test{"world"};
    test.fn("spawn 1", spawn_1);
    test.fn("spawn 2", spawn_2);
    test.fn("spawn 3", spawn_3);

    test.fn("cell can spawn when previous occupant deactivated", cellspawn);
    for (auto i = 0; i < 100; i++)
        test.fn("update extracts deactivated agents", updateextract);
}
}

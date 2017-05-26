#include "../agent_manager.hpp"
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
    
void run() {
    cpptest::Module test{"world"};
    test.fn("spawn 1", spawn_1);
    test.fn("spawn 2", spawn_2);
    test.fn("spawn 3", spawn_3);
}
}

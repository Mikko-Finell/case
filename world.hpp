#ifndef CASE_WORLD
#define CASE_WORLD

#include <algorithm>
#include <functional>
#include <vector>

#include "gc.hpp"
#include "grid.hpp"

namespace CASE {

template<class Cell>
class World {
    using Agent = typename Cell::Agent;

    const int max_agents;
    int live_count = 0;
    GarbageCollector<Agent> gc;

public:
    Agent * agents;
    Grid<Cell> & grid;

    World(const int array_size, Grid<Cell> & _grid)
        : max_agents(array_size), gc{array_size}, grid(_grid)
    {
        agents = new Agent[max_agents];
        for (auto i = 0; i < max_agents; i++)
            agents[i].deactivate();
    }

    ~World() {
        delete [] agents;
    }

    int count() const {
        int n = 0;
        for (auto i = 0; i < max_agents; i++) {
            if (agents[i].active())
                ++n;
        }
        return n;
    }

    Agent * spawn() {
        auto live_count = count();
        if (live_count == max_agents)
            return nullptr;

        return &agents[live_count];
    }

    Code kill(Agent & agent) {
        if (agent.cell != nullptr)
            agent.cell->extract(agent);

        agent.deactivate();
        assert(agent.cell == nullptr);

        live_count = gc.compact(agents);
        return Code::OK;
    }

    void clear() {
        grid.clear();
        for (auto i = 0; i < max_agents; i++)
            agents[i] = Agent{};
    }
};

} // CASE

#endif // WORLD

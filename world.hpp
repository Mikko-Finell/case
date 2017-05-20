#ifndef CASE_WORLD
#define CASE_WORLD

#include <cassert>
#include <deque>

#include "code.hpp"
#include "gc.hpp"

namespace CASE {

template<class Agent, class Grid>
class World {
    const int max_agents;
    std::deque<int> available_indices;
    GarbageCollector<Agent> gc;

    class _mover {
        World<Agent, Grid> & world;
        Agent & agent;

    public:
        _mover(World<Agent, Grid> & w, Agent & a) : world(w), agent(a)
        {}

        Code to(const int x, const int y) {
            return world.try_move(agent, x, y);
        }
    };

public:
    Agent * agents;
    Grid grid;

    World(const int array_size)
        : max_agents(array_size)
        , gc{max_agents}
    {
        agents = new Agent[max_agents];
        for (auto i = 0; i < max_agents; i++) {
            available_indices.push_back(i);
            agents[i].deactivate();
        }
    }

    ~World() {
        delete [] agents;
    }

    int count() {
        //std::cout << "MAX " << max_agents << std::endl;
        //std::cout << "FREE " << available_indices.size() << std::endl;
        //std::cout << "LIVE " << max_agents - available_indices.size() << std::endl;
        gc.compact(agents);
        return max_agents - available_indices.size();
    }

    Code try_move(Agent & agent, const int x, const int y) {
        assert(agent.cell != nullptr);

        auto cell = agent.cell;
        cell->extract(agent);

        if (grid(x, y).insert(agent) == Code::OK) {
            agent.x = wrap(x, grid.columns);
            agent.y = wrap(y, grid.rows);
            return Code::OK;
        }
        else if (cell->insert(agent) == Code::OK)
            return Code::Rejected;
        else
            return Code::InternalError;
    }

    auto try_move(Agent & agent) {
        return _mover{*this, agent};
    }

    Agent * try_spawn(const Agent & agent) {
        assert(available_indices.empty() == false);
        if (available_indices.empty()) {
            std::cout << "Cant spawn because no available slots left" << std::endl;
            return nullptr;
        }

        const auto index = available_indices.front();

        // copy the argument into the array slot at index
        agents[index] = agent;
        auto ptr = &agents[index];

        ptr->activate();
        const auto code = grid(agent.x, agent.y).insert(ptr);
        if (code == Code::OK) {
            available_indices.pop_front();
            return ptr;
        }
        else {
            //std::cout << "Cant spawn because was unable to insert" << std::endl;
            ptr->deactivate();
            return nullptr;
        }
    }

    Code kill(Agent & agent) {
        for (auto i = 0; i < max_agents; i++) {
            if (&agent == &agents[i]) {
                agent.cell->extract(agent.z);
                assert(agent.cell == nullptr);
                agent.deactivate();
                available_indices.push_back(i);
                return Code::OK;
            }
        }
        return Code::NotFound;
    }

    void sort() {
        gc.compact(agents);
        available_indices.clear();
        bool expect_dead = false;
        for (auto i = 0; i < max_agents; i++) {
            if (agents[i].active() == false) {
                available_indices.push_back(i);
                expect_dead = true;
            }
            else {
                if (expect_dead) {
                    assert(true == false);
                }
            }
        }
    }

    void clear() {
        grid.clear();
        available_indices.clear();
        for (auto i = 0; i < max_agents; i++) {
            agents[i] = Agent{};
            agents[i].deactivate();
            available_indices.push_back(i);
        }
    }
};

} // CASE

#endif // WORLD

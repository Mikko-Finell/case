#ifndef CASE_AGENTMANAGER
#define CASE_AGENTMANAGER

#include <cstring>
#include <cassert>
#include <vector>

#include "timer.hpp"

namespace CASE {

template <class Agent>
class AgentManager {
    Agent * agents = nullptr;
    const int max_agents;
    std::vector<int> inactive;

    /*Agent * find_inactive_agent() {
        if (inactive.empty()) {
            for (auto i = max_agents - 1; i >= 0; --i) {
                if (agents[i].active() == false)
                    inactive.push_back(i);
            }
            if (inactive.empty())
                return nullptr;
        }
        const auto i = inactive.back();
        inactive.pop_back();
        return &agents[i];
    }*/

    Agent * _spawn(Agent ** _p) {
        Agent *& pointer = *_p;
        
        assert(pointer < agents || pointer > (agents + max_agents));

        if (inactive.empty()) {
            for (auto i = max_agents - 1; i >= 0; --i) {
                if (agents[i].active() == false)
                    inactive.push_back(i);
            }

            if (inactive.empty()) {
                delete pointer;
                pointer = nullptr;
                return pointer;
            }
        }
        const auto i = inactive.back();
        inactive.pop_back();
        
        if (agents[i].cell != nullptr)
            agents[i].cell->extract(agents[i].z);

        agents[i] = *pointer;
        delete pointer;
        pointer = &agents[i];
        pointer->activate();
        return pointer;
    }

    Agent * _spawn(Agent && agent) {
        if (inactive.empty()) {

            for (auto i = max_agents - 1; i >= 0; --i) {
                if (agents[i].active() == false)
                    inactive.push_back(i);
            }

            if (inactive.empty())
                return nullptr;
        }
        const auto i = inactive.back();
        inactive.pop_back();
        
        if (agents[i].cell != nullptr)
            agents[i].cell->extract(agents[i].z);

        agents[i] = agent;
        agents[i].activate();
        return &agents[i];
    }

public:
    void update() {
        inactive.clear();
        for (auto i = 0; i < max_agents; i++) {
            auto & agent = agents[i];
            if (agent.active())
                agent.update();

            if (agent.active() == false) {
                if (agent.cell != nullptr)
                    agent.cell->extract(agent.z);

                inactive.push_back(i);
            }
        }
    }

    AgentManager(const int max) : max_agents(max)
    {
        clear();
    }

    ~AgentManager() {
        if (agents != nullptr)
            delete [] agents;
        agents = nullptr;
    }

    Agent * spawn(Agent *& pointer) {
        return _spawn(&pointer);
    }

    Agent * spawn(Agent *&& pointer) {
        return _spawn(&pointer);
    }

    Agent * spawn(Agent && agent) {
        return _spawn(std::forward<Agent>(agent));
    }

    int popcount() const {
        auto count = 0;
        for (auto i = 0; i < max_agents; i++)
            count += agents[i].active() ? 1 : 0;
        return count;
    }

    void clear() {
        if (agents != nullptr)
            delete [] agents;
        agents = new Agent[max_agents];
        inactive.resize(max_agents);
        std::iota(inactive.rbegin(), inactive.rend(), 0);
    }
};

} // CASE

#endif // AGENTMANAGER

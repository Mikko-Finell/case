#ifndef CASE_AGENTMANAGER
#define CASE_AGENTMANAGER

#include <cstring>
#include <cassert>
#include <vector>

#include "random.hpp"
#include "timer.hpp"

namespace CASE {

template <class Agent>
class AgentManager {
    Uniform<> random;
    Agent * agents = nullptr;
    const int max_agents;
    std::vector<int> inactive;
    std::vector<int> indices;

    void refresh_inactive() {
        if (inactive.empty()) {
            for (auto i = 0; i < max_agents; i++) {
                auto & agent = agents[i];
                if (agent.active() == false) {
                    inactive.push_back(i);
                    if (agent.cell != nullptr)
                        agent.cell->extract(agent.z);
                }
            }
        }
    }

    Agent * _spawn(Agent ** _p) {
        Agent *& pointer = *_p;
        
        assert(pointer < agents || pointer > (agents + max_agents));

        refresh_inactive();

        if (inactive.empty()) {
            delete pointer;
            pointer = nullptr;
            return pointer;
        }

        const auto i = inactive.back();
        inactive.pop_back();

        agents[i] = *pointer;
        agents[i].activate();
        delete pointer;
        pointer = &agents[i];
        return pointer;
    }

    Agent * _spawn(Agent && agent) {
        refresh_inactive();

        if (inactive.empty())
            return nullptr;

        const auto i = inactive.back();
        inactive.pop_back();

        agents[i] = agent;
        agents[i].activate();
        return &agents[i];
    }

public:
    void update() {
        for (const auto i : random.shuffle(indices)) {
            auto & agent = agents[i];
            if (agent.active())
                agent.update();

            if (agent.active() == false) {
                if (agent.cell != nullptr)
                    agent.cell->extract(agent.z);
            }
        }
    }

    AgentManager(const int max) : max_agents(max)
    {
        indices.resize(max_agents);
        std::iota(indices.begin(), indices.end(), 0);
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
        return max_agents - inactive.size();
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

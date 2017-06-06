#ifndef CASE_AGENTMANAGER
#define CASE_AGENTMANAGER

#include <cstring>
#include <cassert>
#include <vector>
#include <list>

#include "pair.hpp"
#include "random.hpp"
#include "job.hpp"
#include "timer.hpp"

namespace CASE {

class ShuffleJob : public Job {
    Uniform<> random;
    std::vector<int> * indices;

    void execute() override {
        random.shuffle(*indices);
    }

public:
    void upload(std::vector<int> * i) { indices = i; }
};

template <class Agent>
class AgentManager {
    Agent * agents = nullptr;
    const int max_agents;
    std::vector<int> inactive;
    
    ShuffleJob shuffle;
    Pair<std::vector<int>> indices;

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
    AgentManager(const int max) : max_agents(max)
    {
        clear();
        shuffle.thread = std::thread{[this]{ shuffle.run(); }};
    }

    void update() {
        indices.flip();
        shuffle.wait();
        shuffle.upload(&indices.next());
        shuffle.launch();

        for (const auto i : indices.current()) {
            auto & agent = agents[i];
            if (agent.active())
                agent.update();

            if (agent.active() == false) {
                if (agent.cell != nullptr)
                    agent.cell->extract(agent.z);
            }
        }
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

        auto & a = indices.current();
        a.resize(max_agents);
        std::iota(a.begin(), a.end(), 0);

        auto & b = indices.next();
        b.resize(max_agents);
        std::iota(b.begin(), b.end(), 0);

        inactive.resize(max_agents);
        std::iota(inactive.rbegin(), inactive.rend(), 0);
    }
};

} // CASE

#endif // AGENTMANAGER

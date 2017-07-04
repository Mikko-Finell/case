/* Author: Mikko Finell
 * License: Public Domain */

#ifndef CASE_AGENTMANAGER
#define CASE_AGENTMANAGER

#include <cassert>
#include <vector>
#include <list>

#include "pair.hpp"
#include "random.hpp"
#include "job.hpp"

namespace CASE {

class ShuffleJob : public Job {
    Uniform<> random;
    std::vector<int> * indices;

    void execute() override {
        assert(indices != nullptr);
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
    
public:
    AgentManager(const int max) : max_agents(max)
    {
        clear();
        shuffle.thread = std::thread{[this]{ shuffle.run(); }};
    }

    void update() {
#ifndef CASE_DETERMINISTIC
        indices.flip();
        shuffle.wait();
        shuffle.upload(&indices.next());
        shuffle.launch();
#endif
        for (const auto i : indices.current()) {
            if (agents[i].active())
                agents[i].update();
        }
    }

    ~AgentManager() {
        shuffle.terminate();
        if (agents != nullptr)
            delete [] agents;
        agents = nullptr;
    }

    Agent * spawn(Agent && agent) {
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
        if (inactive.empty())
            return nullptr;

        const auto i = inactive.back();
        inactive.pop_back();

        agents[i] = agent;
        agents[i].activate();
        return &agents[i];
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
        a.shrink_to_fit();
        std::iota(a.begin(), a.end(), 0);

        auto & b = indices.next();
        b.resize(max_agents);
        b.shrink_to_fit();
        std::iota(b.begin(), b.end(), 0);

        inactive.resize(max_agents);
        inactive.shrink_to_fit();
        std::iota(inactive.rbegin(), inactive.rend(), 0);
    }
};

} // CASE

#endif // AGENTMANAGER

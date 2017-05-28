#ifndef CASE_AGENTMANAGER
#define CASE_AGENTMANAGER

#include <cstring>
#include <cassert>

namespace CASE {

template <class Agent>
class AgentManager {
    Agent * agents = nullptr;
    const int max_agents;

    Agent * _spawn(Agent ** _p) {
        Agent *& pointer = *_p;
        
        assert(pointer < agents || pointer > (agents + max_agents));
/*#ifdef NDEBUG
        if (pointer < agents || pointer > (agents + max_agents))
            return nullptr;
#endif*/
        for (auto i = 0; i < max_agents; i++) {
            if (agents[i].active() == false) {

                if (agents[i].cell != nullptr)
                    agents[i].cell->extract(agents[i].z);

                std::memmove(&agents[i], pointer, sizeof(Agent));
                delete pointer;
                pointer = &agents[i];
                pointer->activate();
                return pointer;
            }
        }
        delete pointer;
        pointer = nullptr;
        return pointer;
    }

    Agent * _spawn(Agent && agent) {
        Agent * pointer = nullptr;
        for (auto i = 0; i < max_agents; i++) {
            if (agents[i].active() == false) {

                if (agents[i].cell != nullptr)
                    agents[i].cell->extract(agents[i].z);

                agents[i] = agent;
                agents[i].activate();
                pointer = &agents[i];

                break;
            }
        }
        return pointer;
    }

public:
    void update() {
        for (auto i = 0; i < max_agents; i++) {
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
    }
};

} // CASE

#endif // AGENTMANAGER

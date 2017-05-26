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

public:
    AgentManager(const int max) : max_agents(max)
    {
        if (agents != nullptr)
            delete [] agents;
        agents = new Agent[max_agents];
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

    void update() {
        for (auto i = 0; i < max_agents; i++) {
            if (agents[i].active())
                agents[i].update();
        }
        for (auto i = 0; i < max_agents; i++) {
            if (agents[i].active() == false) {
                if (agents[i].cell != nullptr)
                    agents[i].cell->extract(agents[i].z);
            }
        }
    }
};

} // CASE

#endif // AGENTMANAGER

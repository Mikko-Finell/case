/* Author: Mikko Finell
 * License: Public Domain */

#ifndef CASE_CELL
#define CASE_CELL

#include <vector>
#include <SFML/Graphics/Vertex.hpp>

#include "neighbors.hpp"
#include "agent_manager.hpp"

namespace CASE {

template <class T, int LAYERS>
class ZCell {

    static_assert(LAYERS > 0, "ZCell LAYERS must be > 0.");
    T * array[LAYERS];
    AgentManager<T> * manager = nullptr;

public:
    using Agent = T;
    static constexpr int depth = LAYERS;
    int x = 0, y = 0;
    int index = 0;

    ZCell() {
        for (auto i = 0; i < depth; i++)
            array[i] = nullptr;
    }

    ZCell(AgentManager<Agent> & am) {
        set_manager(am);
        for (auto i = 0; i < depth; i++)
            array[i] = nullptr;
    }

    inline void set_manager(AgentManager<Agent> & am) {
        manager = &am;
    }

    void operator=(ZCell & other) {
        for (auto i = 0; i < LAYERS; i++) {
            extract(i);
            insert(other.extract(i));
        }
    }

    Agent * spawn(Agent && agent) {
        assert(manager != nullptr);
        assert(agent.z >= 0);
        assert(agent.z < LAYERS);

        if (array[agent.z] != nullptr) {
            if (array[agent.z]->active())
                return nullptr;
        }

        auto pointer = manager->spawn(std::forward<T>(agent));
        if (pointer == nullptr)
            return nullptr;

        if (insert(*pointer) == nullptr) {
            pointer->deactivate();
            pointer = nullptr;
        }

        return pointer;
    }

    auto neighbors() {
        return Neighbors<ZCell<Agent, depth>>{this};
    }

    Agent * getlayer(const int layer) {
        assert(layer < LAYERS);
        assert(layer >= 0);

        if (array[layer] != nullptr) {
            if (array[layer]->active() == false)
                extract(layer);
        }

        return array[layer];
    }

    inline Agent * operator[](const int layer) {
        return getlayer(layer);
    }

    Agent * insert(Agent & agent) {
        const auto layer = agent.z;

        assert(layer >= 0);
        assert(layer < LAYERS);

        if (array[layer] != nullptr) {
            if (array[layer]->active())
                return nullptr;

            else
                extract(layer);
        }

        if (agent.cell != nullptr)
            agent.cell->extract(layer);

        array[layer] = &agent;
        array[layer]->cell = this;
        array[layer]->activate();

        return array[layer];
    }

    Agent * insert(Agent * agent) {
        if (agent == nullptr)
            return nullptr;
        else
            return insert(*agent);
    }

    Agent * extract(const int layer) {
        assert(layer < LAYERS);
        assert(layer >= 0);

        auto agent = array[layer];
        if (agent != nullptr) {
            agent->cell = nullptr;
            agent->deactivate();
            array[layer] = nullptr;
        }
        return agent;
    }

    void draw(std::vector<sf::Vertex> & vertices) const {
        for (auto i = depth - 1; i >= 0; --i) {
            const auto agent = array[i];
            if (agent != nullptr && agent->active())
                agent->draw(x, y, vertices);
        }
    }

    void clear() {
        for (auto i = 0; i < depth; i++)
            extract(i);
    }

    int popcount() const {
        auto count = 0;
        for (const auto layer : array) {
            if (layer != nullptr && layer->active())
                count++;
        }
        return count;
    }

    inline bool is_empty() const { return popcount() == 0; }
    inline bool is_occupied() const { return !is_empty(); }
};

} // CASE

#endif // CELL

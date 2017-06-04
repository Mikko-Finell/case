#ifndef CASE_CELL
#define CASE_CELL

#include <vector>
#include <SFML/Graphics/Vertex.hpp>

#include "code.hpp"
#include "neighbors.hpp"
#include "agent_manager.hpp"

namespace CASE {

template <class T, int LAYERS>
class ZCell {

    static_assert(LAYERS > 0, "ZCell LAYERS must be > 0.");
    T * array[LAYERS];
    AgentManager<T> * manager = nullptr;

    T * _spawn(T * pointer) {
        if (pointer == nullptr)
            return nullptr;

        const auto code = insert(*pointer);
        if (code == Rejected) {
            pointer->deactivate();
            pointer = nullptr;
        }

        return pointer;
    }

public:

    using Agent = T;
    static constexpr int depth = LAYERS;
    int x = 0, y = 0;
    int index = 0;

    ZCell() {
        for (auto i = 0; i < depth; i++)
            array[i] = nullptr;
    }

    ZCell(AgentManager<Agent> & am) : ZCell{}
    {
        set_manager(am);
    }

    void set_manager(AgentManager<Agent> & am) {
        manager = &am;
    }

    void operator=(ZCell & other) {
        for (auto i = 0; i < LAYERS; i++)
            replace(other.extract(i));
    }

    auto replace(Agent & agent) {
        extract(agent.z);
        return insert(agent);
    }

    Agent * spawn(Agent *& pointer) {
        assert(manager != nullptr);

        return _spawn(manager->spawn(pointer));
    }

    Agent * spawn(Agent *&& pointer) {
        assert(manager != nullptr);

        return _spawn(manager->spawn(pointer));
    }

    Agent * spawn(Agent && agent) {
        assert(manager != nullptr);

        auto pointer = manager->spawn(std::forward<T>(agent));
        return _spawn(pointer);
    }

    auto neighbors() {
        return Neighbors<ZCell>{this};
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

    Agent * operator[](const int layer) {
        return getlayer(layer);
    }

    Code insert(Agent & agent) {
        const auto layer = agent.z;

        if (array[layer] != nullptr) {
            if (array[layer]->active())
                return Rejected;

            else
                extract(layer);
        }

        if (agent.cell != nullptr)
            agent.cell->extract(layer);

        array[layer] = &agent;
        array[layer]->cell = this;
        array[layer]->activate();

        return OK;
    }

    Code insert(Agent * agent) {
        if (agent == nullptr)
            return Rejected;
        else
            return insert(*agent);
    }

    void force_insert(Agent * agent) {
        extract(agent->z);
        array[agent->z] = agent;
        agent->cell = this;
        agent->activate();
    }

    Agent * extract(const int layer) {
        assert(layer < LAYERS);
        assert(layer >= 0);

        auto occupant = array[layer];
        if (occupant != nullptr) {
            occupant->cell = nullptr;
            occupant->deactivate();
            array[layer] = nullptr;
        }
        return occupant;
    }

    void swap_with(ZCell & other) {
        for (auto i = 0; i < LAYERS; i++) {
            auto agent = extract(i);
            insert(other.extract(i));
            other.insert(agent);
        }
    }

    void draw(std::vector<sf::Vertex> & vertices) const {
        for (auto i = depth - 1; i >= 0; --i) {
            const auto occupant = array[i];
            if (occupant != nullptr && occupant->active())
                occupant->draw(x, y, vertices);
        }
    }

    void clear() {
        for (auto i = 0; i < depth; i++)
            extract(i);
    }

    int popcount() const {
        auto count = 0;
        for (const auto layer : array) {
            if (layer != nullptr)
                count++;
        }
        return count;
    }

    inline bool is_empty() const { return popcount() == 0; }
    inline bool is_occupied() const { return !is_empty(); }
};

} // CASE

#endif // CELL

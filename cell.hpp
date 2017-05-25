#ifndef CASE_CELL
#define CASE_CELL

#include <vector>
#include <SFML/Graphics/Vertex.hpp>

#include "code.hpp"
#include "neighbors.hpp"

namespace CASE {

template <class T, int LAYERS>
class ZCell {

    static_assert(LAYERS > 0, "ZCell LAYERS must be > 0.");
    T * array[LAYERS];

public:
    using Agent = T;
    static constexpr int depth = LAYERS;
    int x = 0, y = 0;
    int index = 0;

    ZCell() {
        for (auto i = 0; i < depth; i++)
            array[i] = nullptr;
    }

    void operator=(ZCell & other) {
        for (auto i = 0; i < LAYERS; i++)
            replace(other.extract(i));
    }

    auto replace(Agent & agent) {
        extract(agent.z);
        return insert(agent);
    }

    auto neighbors() {
        return Neighbors<ZCell>{this};
    }

    void draw(std::vector<sf::Vertex> & vertices) const {
        for (auto i = depth - 1; i > -1 ; --i) {
            const auto layer = array[i];
            if (layer != nullptr)
                layer->draw(x, y, vertices);
        }
    }

    Code insert(Agent & agent) {
        const auto layer = agent.z;

        if (array[layer] != nullptr) {
            auto & occupant = *array[layer];
            if (occupant.active()) {
                if (occupant.cell == this)
                    return Rejected;
            }
            else
                extract(layer);
        }

        if (agent.cell != nullptr)
            agent.cell->extract(layer);

        array[layer] = &agent;
        array[layer]->activate();
        array[layer]->cell = this;

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

        auto agent = array[layer];

        if (agent != nullptr) {
            array[layer] = nullptr;
            agent->cell = nullptr;
            agent->deactivate();
        }

        return agent;
    }

    Agent * getlayer(const int layer) {
        assert(layer < LAYERS);
        assert(layer >= 0);

        return array[layer];
    }

    void swap_with(ZCell & other) {
        for (auto i = 0; i < LAYERS; i++) {
            auto agent = extract(i);
            insert(other.extract(i));
            other.insert(agent);
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

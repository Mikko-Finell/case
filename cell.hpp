#ifndef CASE_CELL
#define CASE_CELL

#include "code.hpp"
#include "neighbors.hpp"

namespace CASE {

template <class Agent, int LAYERS>
class ZCell {

    static_assert(LAYERS > 0, "ZCell LAYERS must be > 0.");
    Agent array[LAYERS];

public:
    static constexpr int depth = LAYERS;
    int index = 0;

    ZCell() {}

    void operator=(ZCell & other) {
        for (auto i = 0; i < LAYERS; i++)
            replace(other.extract(i));
    }

    auto neighbors() {
        return Neighbors<ZCell>{this};
    }

    template <class Grid>
    void update(Grid & grid) {
        for (auto & agent : array) {
            if (agent.active())
                agent.update(grid);
        }
    }

    void draw(std::vector<sf::Vertex> & vertices) const {
        for (auto i = depth - 1; i > -1 ; --i) {
            if (array[i].active())
                array[i].draw(vertices);
        }
    }

    Agent & insert(const Agent & agent) {
        assert(agent.z < LAYERS);
        assert(agent.z >= 0);

        array[agent.z] = agent;
        array[agent.z].activate();
        array[agent.z].cell = this;

        return array[agent.z];
    }

    Agent extract(const int layer) {
        assert(layer < LAYERS);
        assert(layer >= 0);

        auto & agent = array[layer];
        agent.deactivate();
        agent.cell = nullptr;
        return agent;
    }

    void replace(const Agent & agent) {
        extract(agent.z);
        return insert(agent);
    }

    Agent * getlayer(const int layer) {
        assert(layer < LAYERS);
        assert(layer >= 0);
        if (array[layer].active())
            return &array[layer];
        else
            return nullptr;
    }

    void swap_with(ZCell & other) {
        for (auto i = 0; i < LAYERS; i++) {
            auto agent = extract(i);
            insert(other.extract(i));
            other.insert(agent);
        }
    }

    void clear() {
        for (auto & agent : array)
            agent.deactivate();
    }

    int popcount() const {
        auto count = 0;
        for (const auto & agent : array) {
            if (agent.active())
                count++;
        }
        return count;
    }

    inline bool is_empty() const { return popcount() == 0; }
    inline bool is_occupied() const { return !is_empty(); }
};

} // CASE

#endif // CELL

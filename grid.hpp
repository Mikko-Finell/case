#ifndef CASE_GRID
#define CASE_GRID

#include <cassert>

#include "code.hpp"
#include "index.hpp"
#include "random.hpp"
#include "log.hpp"
#include "timer.hpp"

namespace CASE {

template<class Cell>
class Grid {
    using Agent = typename Cell::Agent;

    int columns = 0;
    int rows = 0;
    std::mt19937 rng;

    inline Cell & get(const int x, const int y) {
        assert(_index(x, y) >= 0);
        assert(_index(x, y) < columns * rows);
        return cells[index(wrap(x, columns), wrap(y, rows), columns)];
    }

public:
    Cell * cells = nullptr;
    Agent * agents = nullptr;

    ~Grid() {
        if (cells != nullptr)
            delete [] cells;
        cells = nullptr;
        if (agents != nullptr)
            delete [] agents;
        agents = nullptr;
    }

    Grid() {
        std::random_device rd;
        rng.seed(rd());
    }

    Grid(const int cols, const int _rows) {
        init(cols, _rows);
    }

    void init(const int cols, const int _rows) {
        assert(cols >= 1);
        assert(_rows >= 1);

        columns = cols;
        rows = _rows;

        if (cells != nullptr)
            delete [] cells;
        cells = new Cell[cell_count()];

        int index = 0;
        for (auto y = 0; y < rows; y++) {
            for (auto x = 0; x < columns; x++) {
                auto & cell = get(x, y);
                cell.x = x;
                cell.y = y;
                cell.index = index++;
            }
        }
    
        // init agents
        if (agents != nullptr)
            delete [] agents;

        agents = new Agent[max_agents()];
    }

    void update(const int subset) {
        for (auto i = 0; i < max_agents(); i++) {
            if (agents[i].active())
                agents[i].update(*this);
        }
    }

    void kill(Agent & agent) {
        agent.deactivate();
        agent.cell->extract(agent.z);
    }

    Agent * spawn(Agent & agent, Cell & cell) {
        for (int i = 0; i < max_agents(); i++) {
            if (agents[i].active() == false) {
                agents[i] = agent;
                if (agents[i].cell != nullptr)
                    agents[i].cell->extract(agent.z);

                if (cell.insert(agents[i]) == OK) {
                    agents[i].activate();
                    return &agents[i];
                }
                else
                    return nullptr;
            }
        }
        return nullptr;
    }

    Agent * spawn(Agent & agent, const int x, const int y) {
        auto & cell = get(x, y);
        return spawn(agent, cell);
    }

    Cell & operator()(const int x, const int y) {
        return get(x, y);
    }

    const Cell & operator()(const int x, const int y) const {
        return get(x, y);
    }

    void clear() {
        for (auto i = 0; i < cell_count(); i++)
            cells[i].clear();
    }

    inline int cell_count() const {
        return rows * columns;
    }

    inline int agent_count() const {
        int count = 0;
        for (int i = 0; i < max_agents(); i++) {
            if (agents[i].active())
                ++count;
        }
        return count;
    }

    inline int max_agents() const {
        return cell_count() * Cell::depth;
    }
};

} // CASE

#endif // GRID

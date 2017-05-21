#ifndef CASE_GRID
#define CASE_GRID

#include <cassert>

#include "code.hpp"
#include "index.hpp"
#include "random.hpp"

namespace CASE {

template<class Cell>
class Grid {
    int columns = 0;
    int rows = 0;
    std::mt19937 rng;
    std::vector<int> indices;

    inline int _index(const int x, const int y) {
        assert(columns > 0);
        assert(rows > 0);
        return index(wrap(x, columns), wrap(y, rows), columns);
    }

    inline Cell & get(const int x, const int y) {
        assert(_index(x, y) >= 0);
        assert(_index(x, y) < columns * rows);
        return cells[_index(x, y)];
    }

public:
    Cell * cells = nullptr;

    Grid() 
    {
        std::random_device rd;
        rng.seed(rd());
    }

    Grid(const int cols, const int _rows)
    {
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

        for (auto i = 0; i < cell_count(); i++) {
            cells[i].index = i;
            indices.push_back(i);
        }
    }

    void update(const int subset) {
        assert(cells != nullptr);
        assert(cell_count() != 0);
        assert(indices.size() == cell_count());

        std::shuffle(indices.begin(), indices.end(), rng);
        const auto size = std::min(subset, cell_count());

        //for (int i = 0; i < size; i++)
        //for (const auto i : indices)
        for (auto i = 0 ; i < cell_count() ; i++)
            cells[i].update(*this);
    }

    ~Grid() {
        if (cells != nullptr)
            delete [] cells;
        cells = nullptr;
    }

    template <class Agent>
    Code move(Agent * _agent, const int x, const int y) {
        auto agent = *_agent;

        const auto layer = agent.z;
        const auto ax = agent.x, ay = agent.y;
        auto & target_cell = get(ax + x, ay + y);
        auto & source_cell = *_agent->cell;

        if (target_cell.getlayer(layer) == nullptr) {

            source_cell.extract(layer);
            auto & inserted_agent = target_cell.insert(agent);

            inserted_agent.x = ax + x;
            inserted_agent.y = ay + y;

            return OK;
        }
        else
            return Rejected;
    }

    template <class Agent>
    Code spawn(Agent & agent, const int x, const int y) {
        auto & cell = get(x, y);
        if (cell.getlayer(agent.z) == nullptr) {
            auto & _agent = cell.insert(agent);
            _agent.setpos(x, y);
            return OK;
        }
        else
            return Rejected;
    }

    template <class Agent>
    Code spawn(Agent & agent) {
        return spawn(agent, agent.x, agent.y);
    }

    Cell & operator()(const int x, const int y) {
        return get(x, y);
    }

    const Cell & operator()(const int x, const int y) const {
        return get(x, y);
    }

    Cell & operator()(const CASE::Random & rng) {
        return cells[rng(0, columns * rows - 1)];
    }

    void clear() {
        for (auto i = 0; i < columns * rows; i++)
            cells[i].clear();
    }

    inline int cell_count() const {
        return rows * columns;
    }
};

} // CASE

#endif // GRID

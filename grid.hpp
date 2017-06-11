/* Author: Mikko Finell
 * License: Public Domain */

#ifndef CASE_GRID
#define CASE_GRID

#include <cassert>

#include "index.hpp"
#include "agent_manager.hpp"

namespace CASE {

template<class Cell>
class Grid {
    using Agent = typename Cell::Agent;

    int columns = 0;
    int rows = 0;

    inline Cell & get(const int x, const int y) {
        return cells[index(wrap(x, columns), wrap(y, rows), columns)];
    }

public:
    Cell * cells = nullptr;

    ~Grid() {
        if (cells != nullptr)
            delete [] cells;
        cells = nullptr;
    }

    Grid() {}

    Grid(const int cols, const int _rows, AgentManager<Agent> & manager) {
        init(cols, _rows, manager);
    }

    void init(const int cols, const int _rows, AgentManager<Agent> & manager) {
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
                cell.set_manager(manager);
            }
        }
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
};

} // CASE

#endif // GRID

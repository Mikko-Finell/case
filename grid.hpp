#ifndef CASE_GRID
#define CASE_GRID

#include <cassert>
#include "index.hpp"
#include "random.hpp"

namespace CASE {

template<class Cell>
class Grid {
    Cell * cells = nullptr;

    inline int _index(int x, int y) {
        assert(columns > 0);
        assert(rows > 0);
        return index(wrap(x, columns), wrap(y, rows), columns);
    }

public:
    int columns = 0;
    int rows = 0;

    Grid() 
    {
    }

    Grid(const Grid & other) {
        init(other.columns, other.rows);
    }

    Grid(const int cols, const int _rows)
    {
        init(cols, _rows);
    }

    void operator=(const Grid & other) {
        init(other.columns, other.rows);
    }

    void init(const int cols, const int _rows) {
        assert(cols >= 1);
        assert(_rows >= 1);

        columns = cols;
        rows = _rows;

        if (cells != nullptr)
            delete [] cells;
        cells = new Cell[columns * rows];

        static const int range[3] = {-1, 0, 1};

        // for every cell
        for (auto row = 0; row < rows; row++) {
            for (auto col = 0; col < columns; col++) {
                // for each of that cells neighbors
                auto & cell = cells[_index(col, row)];
                for (const auto y : range) {
                    for (const auto x : range) {
                        auto & neighbor = cells[_index(col + x, row + y)];
                        cell.neighbors.assign_cell(neighbor, x, y);
                    }
                }
            }
        }
    }

    ~Grid() {
        if (cells != nullptr)
            delete [] cells;
        cells = nullptr;
    }

    Cell & operator()(const int x, const int y) {
        assert(_index(x, y) >= 0);
        assert(_index(x, y) < columns * rows);
        return cells[_index(x, y)];
    }

    const Cell & operator()(const int x, const int y) const {
        assert(_index(x, y) >= 0);
        assert(_index(x, y) < columns * rows);
        return cells[_index(x, y)];
    }

    Cell & operator()(const CASE::Random & rng) {
        return cells[rng(0, columns * rows - 1)];
    }

    void clear() {
        for (auto i = 0; i < columns * rows; i++)
            cells[i].clear();
    }
};

} // CASE

#endif // GRID

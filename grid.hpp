#ifndef CASE_GRID
#define CASE_GRID

#include <cassert>
#include "index.hpp"
#include "random.hpp"

namespace CASE {

template<class T>
class Grid {
    T * cells;
    int columns = 0;
    int rows = 0;

    inline int _index(int x, int y) {
        assert(columns > 0);
        assert(rows > 0);
        return index(wrap(x, columns), wrap(y, rows), columns);
    }

public:
    Grid(const int _cols, const int _rows) : columns(_cols), rows(_rows)
    {
        assert(_cols >= 1);
        assert(_rows >= 1);

        cells = new T[columns * rows];
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
        delete [] cells;
    }

    T & operator()(const int x, const int y) {
        assert(_index(x, y) >= 0);
        assert(_index(x, y) < columns * rows);
        return cells[_index(x, y)];
    }

    const T & operator()(const int x, const int y) const {
        assert(_index(x, y) >= 0);
        assert(_index(x, y) < columns * rows);
        return cells[_index(x, y)];
    }

    T & operator()(const CASE::Random & rng) {
        return cells[rng(0, columns * rows - 1)];
    }

    void clear() {
        for (auto i = 0; i < columns * rows; i++)
            cells[i].clear();
    }
};

} // CASE

#endif // GRID

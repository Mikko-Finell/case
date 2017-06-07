/* Author: Mikko Finell
 * License: Public Domain */

#ifndef CASE_NEIGHBORS
#define CASE_NEIGHBORS

#include <array>
#include <cassert>

#include "index.hpp"

namespace CASE {

template <class Cell>
class CAdjacent {
    const Cell * self = nullptr;

public:
    CAdjacent(const Cell * cell) : self(cell)
    {}

    CAdjacent(const Cell * cell, const int columns, const int rows)
        : self(cell)
    {
        CAdjacent::columns = columns;
        CAdjacent::rows = rows;
    }

    const Cell & operator()(const int x, const int y) const {
        assert(self != nullptr);
        assert(columns != 0 && rows != 0);

        const int i = self->index;
        const int gx = (i % columns) + x;
        const int gy = (i / columns) + y;
        return *(self - i + index(wrap(gx, columns), wrap(gy, rows), columns));
    }

    static int columns;
    static int rows;
};

template <class T>
int CAdjacent<T>::columns = 0;
template <class T>
int CAdjacent<T>::rows = 0;

template <class Cell>
class Adjacent {
    int columns = 0, rows = 0;
    Cell * self = nullptr;

public:
    Adjacent(Cell * cell, const int c, const int r)
        : columns(c), rows(r), self(cell)
    {
        assert(cell != nullptr);
        assert(columns != 0 && rows != 0);
    }

    Cell & operator()(const int x, const int y) {
        assert(self != nullptr);

        const int i = self->index;
        const int gx = (i % columns) + x;
        const int gy = (i / columns) + y;
        const auto offset = index(wrap(gx, columns), wrap(gy, rows), columns);
        return *(self - i + offset);
    }
};

template <class Cell>
class Neighbors {

    inline int index(const int x, const int y) const {
        assert(x > -2 && x < 2);
        assert(y > -2 && y < 2);
        return 3 * y + x + 4;
    }

    Adjacent<Cell> adjacent;
    
public:
    Neighbors(Cell * cell) : adjacent{cell, columns, rows}
    {
        assert(cell != nullptr);
        assert(columns != 0 && rows != 0);
    }

    auto cells() {
        std::array<Cell *, 9> array{{nullptr}};
        static const int range[3] = {-1, 0, 1};
        for (const auto y : range) {
            for (const auto x : range)
                array[index(x, y)] = &adjacent(x, y);
        }
        return array;
    }

    int popcount() const {
        int count = 0;
        static const int range[3] = {-1, 0, 1};
        for (const auto y : range) {
            for (const auto x : range)
                count += adjacent(x, y).popcount();
        }
        return count;
    }

    inline Cell & operator()(const int x, const int y) {
        return adjacent(x, y);
    }

    static int columns;
    static int rows;
};

template <class T>
int Neighbors<T>::columns = 0;
template <class T>
int Neighbors<T>::rows = 0;

} // CASE

#endif // NEIGHBORS

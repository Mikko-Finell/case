#ifndef CASE_GRID
#define CASE_GRID

#include <cassert>

namespace CASE {

namespace impl {

inline int wrap(const int n, const int max) {
    assert(max > 0);
    if (n < 0)
        return ((n % max) + max) % max;
    else
        return n % max;
}

inline int index(const int x, const int y, const int size) {
    assert(size > 0);
    return y * size + x;
}

}

template<class T>
class Grid {
    T * cells;
    int columns = 0;
    int rows = 0;

    inline int index(int x, int y) {
        assert(columns > 0);
        assert(rows > 0);
        return impl::index(impl::wrap(x, columns), impl::wrap(y, rows), columns);
    }
public:

    Grid(const int _cols, const int _rows) : columns(_cols), rows(_rows)
    {
        if (_cols < 1)
            throw std::invalid_argument{"Columns must be >= 1"};
        if (_rows < 1)
            throw std::invalid_argument{"Rows must be >= 1"};

        cells = new T[columns * rows];

        // for every cell
        for (auto row = 0; row < rows; row++) {
            for (auto col = 0; col < columns; col++) {
                // for each of that cells neighbors
                auto & cell = cells[index(col, row)];
                for (auto y : {-1, 0, 1}) {
                    for (auto x : {-1, 0, 1}) {
                        assert(cell.neighbors.ptr(x, y) == nullptr);

                        auto & neighbor = cells[index(col + x, row + y)];
                        cell.neighbors.set(x, y, neighbor);

                        assert(cell.neighbors.ptr(x, y) != nullptr);
                    }
                }
            }
        }
    }

    ~Grid() {
        delete [] cells;
    }

    T & operator()(const int x, const int y) {
        assert(index(x, y) >= 0);
        assert(index(x, y) < columns * rows);
        return cells[index(x, y)];
    }

    const T & operator()(const int x, const int y) const {
        assert(index(x, y) >= 0);
        assert(index(x, y) < columns * rows);
        return cells[index(x, y)];
    }
};

} // CASE

#endif // GRID

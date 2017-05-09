#ifndef CASE_NEIGHBORS
#define CASE_NEIGHBORS

#include <cassert>

namespace CASE {

template<class T>
class Neighbors {

    inline int index(const int x, const int y) const {
        assert(x > -2 && x < 2);
        assert(y > -2 && y < 2);
        return 3 * y + x + 4; // row-major order, (y+1) * row_length + (x+1)
    }

    template<class U>
    class _inserter {
        Neighbors<T> & neighbors;
        U * element;

    public:
        _inserter(Neighbors<T> & n, U * u) : neighbors(n), element(u)
        {}

        void at(const int x, const int y) {
            neighbors.insert(element, x, y);
        }
    };

    class _transplanter {
        Neighbors<T> & neighbors;
        int _x = 0, _y = 0;

    public:
        _transplanter(Neighbors<T> & n) : neighbors(n)
        {}

        _transplanter & from(const int x, const int y) {
            _x = x;
            _y = y;
            return *this;
        }

        void to(const int x, const int y) {
            neighbors.transplant(_x, _y, x, y);
        }
    };

    class _swapper {
        Neighbors<T> & neighbors;
        const int _x, _y;

    public:
        _swapper(Neighbors<T> & n, const int x, const int y)
            : neighbors(n), _x(x), _y(y)
        {}

        void with(const int x, const int y) {
            neighbors.swap(x, y, _x, _y);
        }
    };

    T * cells[9] = {
        nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr
    };

    decltype(auto) _extract(const int x, const int y) {
        return cells[index(x, y)]->extract();
    }

    template<class U>
    void _insert(U * u, const int x, const int y) {
        cells[index(x, y)]->insert(u);
    }

public:
    Neighbors() {}

    void assign_cell(T & t, const int x, const int y) {
        cells[index(x, y)] = &t;
    }

    decltype(auto) operator()(const int x, const int y) {
        return cells[index(x, y)]->get();
    }

    decltype(auto) operator()(const int x, const int y) const {
        return cells[index(x, y)]->get();
    }

    decltype(auto) extract(const int x, const int y) {
        return cells[index(x, y)]->extract();
    }

    template<class U>
    void insert(U & u, const int x, const int y) {
        cells[index(x, y)]->insert(u);
    }

    template<class U>
    _inserter<U> insert(U & u) {
        return {*this, &u};
    }

    void transplant(const int sx, const int sy, const int tx, const int ty) {
        _insert(_extract(sx, sy), tx, ty);
    }

    _transplanter transplant() {
        return {*this};
    }

    void swap(const int ax, const int ay, const int bx, const int by) {
        const auto a = _extract(ax, ay);
        transplant(bx, by, ax, ay);
        _insert(a, bx, by);
    }

    _swapper swap(const int x, const int y) {
        return {*this, x, y};
    }

    void clear(const int x, const int y) {
        cells[index(x, y)]->clear();
    }

    void clear() {
        for (auto i = 0; i < 9; i++)
            cells[i]->clear();
    }

    bool cell_is_empty(const int x, const int y) const {
        return cells[index(x, y)]->inhabitants() == 0;
    }

    bool cell_is_occupied(const int x, const int y) const {
        return !cell_is_empty(x, y);
    }

    int population() const {
        int count = 0;
        for (auto i = 0; i < 9; i++)
            count += cells[i]->inhabitants();

        return count;
    }
};

} // CASE

#endif // NEIGHBORS

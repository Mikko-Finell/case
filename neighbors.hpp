#ifndef CASE_NEIGHBORS
#define CASE_NEIGHBORS

#include <cassert>
#include "code.hpp"
#include "index.hpp"

namespace CASE {

namespace neighbors {

template<class T>
class OnGrid {

    // fast index: since we know neighbors only access adjacent cells in a
    // 3 by 3 matrix we can use this optimal conversion function.
    // returns index in row-major order, computed (y+1) * row_length + (x+1)
    inline int index(const int x, const int y) const {
        assert(x > -2 && x < 2);
        assert(y > -2 && y < 2);
        return 3 * y + x + 4;
    }

    template<class U>
    class _inserter {
        OnGrid<T> & neighbors;
        U * element;

    public:
        _inserter(OnGrid<T> & n, U * u) : neighbors(n), element(u)
        {}

        auto at(const int x, const int y) {
            return neighbors.insert(element, x, y);
        }
    };

    class _transplanter {
        OnGrid<T> & neighbors;
        int _x = 0, _y = 0;

    public:
        _transplanter(OnGrid<T> & n) : neighbors(n)
        {}

        _transplanter & from(const int x, const int y) {
            _x = x;
            _y = y;
            return *this;
        }

        auto to(const int x, const int y) {
            return neighbors.transplant(_x, _y, x, y);
        }
    };

    class _swapper {
        OnGrid<T> & neighbors;
        const int _x, _y;

    public:
        _swapper(OnGrid<T> & n, const int x, const int y)
            : neighbors(n), _x(x), _y(y)
        {}

        auto with(const int x, const int y) {
            return neighbors.swap(x, y, _x, _y);
        }
    };

    T * cells[9] = {
        nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr
    };

public:
    OnGrid() {}

    void assign_cell(T & t, const int x, const int y) {
        cells[index(x, y)] = &t;
    }

    auto operator()(const int x, const int y) {
        return cells[index(x, y)]->get();
    }

    auto operator()(const int x, const int y) const {
        return cells[index(x, y)]->get();
    }

    T & cell(const int x, const int y) {
        return *cells[index(x, y)];
    }

    auto extract(const int x, const int y) {
        return cells[index(x, y)]->extract();
    }

    template<class U>
    Code insert(U * u, const int x, const int y) {
        return cells[index(x, y)]->insert(u);
    }

    template<class U>
    Code insert(U & u, const int x, const int y) {
        return insert(&u, x, y);
    }

    template<class U>
    _inserter<U> insert(U * u) {
        return {*this, u};
    }

    template<class U>
    _inserter<U> insert(U & u) {
        return insert(&u);
    }

    Code transplant(const int ax, const int ay, const int bx, const int by) {
        if (ax == bx && ay == by)
            return Code::Rejected;

        auto & a = *cells[index(ax, ay)];
        auto & b = *cells[index(bx, by)];
        b.clear();
        b = a;

        return Code::OK;
    }

    _transplanter transplant() {
        return {*this};
    }

    auto swap(const int ax, const int ay, const int bx, const int by) {
        if (ax == bx && ay == by)
            return Code::OK;

        auto & a = *cells[index(ax, ay)];
        auto & b = *cells[index(bx, by)];
        a.swap_with(b);

        return Code::OK;

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
        return cells[index(x, y)]->popcount() == 0;
    }

    bool cell_is_occupied(const int x, const int y) const {
        return !cell_is_empty(x, y);
    }

    int popcount() const {
        int count = 0;
        for (auto i = 0; i < 9; i++)
            count += cells[i]->popcount();

        return count;
    }
};

template<class T>
class Direct {
    const int columns = 0, rows = 0;
    const T * self = nullptr;

public:
    Direct(const T * t, const int c, const int r)
        : columns(c), rows(r), self(t)
    {}

    const T & operator()(const int x, const int y) const {
        const int i = self->index;
        const int gx = (i % columns) + x;
        const int gy = (i / columns) + y;
        return *(self - i + index(wrap(gx, columns), wrap(gy, rows), columns));
    }
};

} // neighbors

template<class T>
using Neighbors = neighbors::OnGrid<T>;

} // CASE

#endif // NEIGHBORS

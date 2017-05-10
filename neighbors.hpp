#ifndef CASE_NEIGHBORS
#define CASE_NEIGHBORS

#include <cassert>
#include "code.hpp"

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

        auto at(const int x, const int y) {
            return neighbors.insert(element, x, y);
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

        auto to(const int x, const int y) {
            return neighbors.transplant(_x, _y, x, y);
        }
    };

    class _swapper {
        Neighbors<T> & neighbors;
        const int _x, _y;

    public:
        _swapper(Neighbors<T> & n, const int x, const int y)
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
    Neighbors() {}

    void assign_cell(T & t, const int x, const int y) {
        cells[index(x, y)] = &t;
    }

    auto operator()(const int x, const int y) {
        return cells[index(x, y)]->get();
    }

    auto operator()(const int x, const int y) const {
        return cells[index(x, y)]->get();
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
        if (insert(cells[index(ax, ay)]->get(), bx, by) != Code::OK)
            return Code::Rejected;
        else
            extract(ax, ay);
        return Code::OK;
    }

    _transplanter transplant() {
        return {*this};
    }

    auto swap(const int ax, const int ay, const int bx, const int by) {
        auto b = extract(bx, by);
        if (transplant(ax, ay, bx, by) == Code::OK) {
            // a was successfully moved to b
            if (insert(b, ax, ay) == Code::OK)
                return Code::OK;
            else {
                // b cannot be moved to a. Try recover, else emit InternalError
                if (transplant(bx, by, ax, ay) != Code::OK)
                    return Code::InternalError;
            }
        }
        if (insert(b, bx, by) == Code::OK)
            // b could not be swapped with a but original state was recovered
            return Code::Rejected;
        else
            // b is extracted but cannot be reinserted
            return Code::InternalError;
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

} // CASE

#endif // NEIGHBORS

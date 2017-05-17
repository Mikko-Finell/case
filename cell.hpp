#ifndef CASE_CELL
#define CASE_CELL

#include <iostream>

#include "code.hpp"
#include "neighbors.hpp"

namespace CASE {

template<class T>
class CellInterface {
public:
    virtual Code insert(T * t)         = 0;
    virtual Code insert(T & t)         { return insert(&t); }
    virtual T * extract()              = 0;
    virtual T * get()                  = 0;
    virtual Code replace(T * a, T * b) = 0;
    virtual void clear()               = 0;
    virtual int popcount()       const = 0;
    virtual bool is_empty()      const { return popcount() == 0; }
    virtual bool is_occupied()   const { return !is_empty(); }
    virtual ~CellInterface()           {}
};

template<class T, int SIZE>
class ZCell {
    static_assert(SIZE > 0, "ZCell SIZE must be > 0.");
    mutable T * array[SIZE];

    void refresh() const {
        for (auto i = 0; i < SIZE; i++) {
            if (array[i] != nullptr) {
                auto t_ptr = array[i];

                if (t_ptr->active() == false)
                    array[i] = nullptr;

                else if (t_ptr->cell != this)
                    array[i] = nullptr;
            }
        }
    }

    Code _insert(T * t, const int layer) {
        if (t == nullptr)
            return Code::Rejected;
        if (array[layer] != nullptr)
            return Code::Rejected;
        if (t->cell != nullptr && t->cell != this)
            return Code::Rejected;

        t->cell = this;
        array[layer] = t;
        return Code::OK;
    }

    T * _extract(const int layer) {
        T * t = array[layer];
        array[layer] = nullptr;
        if (t != nullptr)
            t->cell = nullptr;
        return t;
    }

    T * _get(const int layer) {
        return array[layer];
    }

public:
    CASE::Neighbors<ZCell<T, SIZE>> neighbors;

    ZCell() {
        for (auto i = 0; i < SIZE; i++)
            array[i] = nullptr;
    }

    void operator=(ZCell & other) {
        for (auto i = 0; i < SIZE; i++)
            replace(i, other.extract(i)); // ignore rejects
    }

    void swap_with(ZCell & other) {
        for (auto i = 0; i < SIZE; i++) {
            auto a = extract(i);
            insert(other.extract(i), i); // ignore rejects
            other.insert(a, i);
        }
    }

    Code insert(T * t, const int layer = 0) {
        assert(layer >= 0);
        assert(layer < SIZE);

        refresh();
        return _insert(t, layer);
    }

    Code insert(T & t, const int layer = 0) {
        return insert(&t, layer);
    }

    T * extract(const int layer) {
        assert(layer >= 0);
        assert(layer < SIZE);

        refresh();
        return _extract(layer);
    }

    T * extract() {
        refresh();
        for (auto i = 0; i < SIZE; i++) {
            if (array[i] != nullptr)
                return _extract(i);
        }
        return nullptr;
    }

    T * get(const int layer = 0) {
        assert(layer >= 0);
        assert(layer < SIZE);

        refresh();
        return _get(layer);
    }
    
    Code replace(const int layer, T * t) {
        assert(layer >= 0);
        assert(layer < SIZE);

        _extract(layer);
        const auto code = _insert(t, layer);
        refresh();
        return code;
    }

    Code replace(const int layer, T & t) {
        return replace(layer, &t);
    }

    Code replace(T * a, T * b) {
        assert(a != b);
        assert(b != nullptr);

        for (auto i = 0; i < SIZE; i++) {
            if (array[i] == a)
                return replace(i, b);
        }
        return Code::NotFound;
    }

    Code replace(T & a, T & b) {
        return replace(&a, &b);
    }

    void clear() {
        for (auto i = 0; i < SIZE; i++)
            _extract(i);
    }

    int popcount() const {
        refresh();
        auto count = 0;
        for (auto i = 0; i < SIZE; i++) {
            if (array[i] != nullptr)
                count++;
        }
        return count;
    }

    inline bool is_empty() const { return popcount() == 0; }
    inline bool is_occupied() const { return !is_empty(); }
};

template<class T>
using SimpleCell = ZCell<T, 1>;

} // CASE

#endif // CELL

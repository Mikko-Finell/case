#ifndef CASE_CELL
#define CASE_CELL

#include "code.hpp"
#include "neighbors.hpp"

namespace CASE {

template<class T, int LAYERS>
class ZCell {

    static_assert(LAYERS > 0, "ZCell LAYERS must be > 0.");
    mutable T * array[LAYERS];

    void refresh(const int layer) const {
        auto t_ptr = array[layer];
        if (t_ptr != nullptr) {
            if (t_ptr->active() == false)
                array[layer] = nullptr;

            if (t_ptr->cell != this)
                array[layer] = nullptr;
        }
    }

    void refresh() const {
        for (auto i = 0; i < LAYERS; i++)
            refresh_layer(i);
    }

    Code _insert(T * t, const int layer) {
        refresh_layer(layer);

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
        refresh(layer);

        T * t = array[layer];
        array[layer] = nullptr;
        if (t != nullptr)
            t->cell = nullptr;
        return t;
    }

    T * _get(const int layer) {
        refresh(layer);
        return array[layer];
    }

public:
    using Agent = T;
    OnGrid<ZCell<T, LAYERS>> neighbors;
    static constexpr int depth = LAYERS;

    ZCell() {
        for (auto i = 0; i < LAYERS; i++)
            array[i] = nullptr;
    }

    void operator=(ZCell & other) {
        for (auto i = 0; i < LAYERS; i++)
            replace(other.extract(i)); // ignore rejects
    }

    Code insert(T * t) {
        assert(t->z < LAYERS);

        return _insert(t, t->z);
    }

    Code insert(T & t) {
        return insert(&t);
    }

    T * extract(const int layer) {
        assert(layer >= 0);
        assert(layer < LAYERS);

        refresh();
        return _extract(layer);
    }

    Code extract(T & t) {
        if (array[t.z] == &t) {
            extract(t.z);
            return Code::OK;
        }
        return Code::NotFound;
    }

    T * extract_top() {
        for (auto i = 0; i < LAYERS; i++) {
            if (array[i] != nullptr)
                return _extract(i);
        }
        return nullptr;
    }

    T * getlayer(const int layer) {
        return get(layer);
    }

    T * get(const int layer = 0) {
        assert(layer >= 0);
        assert(layer < LAYERS);

        return _get(layer);
    }
    
    void swap_with(ZCell & other) {
        for (auto i = 0; i < LAYERS; i++) {
            auto a = extract(i);
            insert(other.extract(i)); // ignore rejects
            other.insert(a);
        }
    }

    Code replace(T * t) {
        _extract(t->z);
        return _insert(t, t->z);
    }

    void clear() {
        for (auto i = 0; i < LAYERS; i++)
            _extract(i);
    }

    int popcount() const {
        refresh();
        auto count = 0;
        for (auto i = 0; i < LAYERS; i++) {
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

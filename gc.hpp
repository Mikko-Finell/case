#ifndef CASE_GC
#define CASE_GC

#include <cassert>
#include <vector>
#include <algorithm>

namespace CASE {

template<class T>
class GarbageCollector {
    T * current, * next;
    const int array_size;
    std::vector<int> live;
    std::vector<int> dead;

    inline void move(const int source, const int target) {
        assert(target < source);
        assert(target >= 0);
        assert(target < array_size);
        assert(source > 0);
        assert(source < array_size);
        assert(current + target < current + array_size - 1);
        assert(next + target < next + array_size - 1);
        assert(current + source < current + array_size);
        assert(next + source < next + array_size);

        current[target] = current[source];
        current[source].deactivate();
        next[target] = next[source];
        next[source].deactivate();
    }

public:
    GarbageCollector(T * a, T * b, const int size)
        : current(a), next(b), array_size(size)
    {
        assert(a != nullptr);
        assert(b != nullptr);
        assert(a != b);
        if (a < b)
            assert(a + size <= b);
        else
            assert(a >= b + size);
        assert(size >= 2);
    }

    void copy_and_compact() {
        live.clear();
        dead.clear();

        for (auto i = 0; i < array_size; i++) {
            if (current[i].active())
                live.push_back(i);
            else
                dead.push_back(i);
        }

        const auto live_count = live.size(), dead_count = dead.size();
        if (live_count == 0 || dead_count == 0)
            return;

        std::reverse(live.begin(), live.end());
        const auto count = std::min(live_count, dead_count);
        for (std::size_t i = 0; i < count; i++) {
            if (live[i] < dead[i])
                break;
            move(live[i], dead[i]);
        }
    }

    inline std::size_t count() const {
        return live.size();
    }
};

template<class T>
using GC = GarbageCollector<T>;

}

#endif

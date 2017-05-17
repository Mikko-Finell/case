#ifndef CASE_GC
#define CASE_GC

#include <cassert>
#include <vector>
#include <algorithm>

namespace CASE {

template<class T>
class GarbageCollector {

    T * objects = nullptr;
    const int array_size;
    std::vector<int> live;
    std::vector<int> dead;

    inline void move(const int source, const int target) {
        assert(target < source);
        assert(target >= 0);
        assert(target < array_size);
        assert(source > 0);
        assert(source < array_size);
        assert(objects + target < objects + array_size - 1);
        assert(objects + source < objects + array_size);

        // copy from source to target
        objects[target] = objects[source];

        // in cell, replace source with target
        auto cell = objects[target].cell;
        assert(cell != nullptr);
        assert(objects[source].cell != nullptr);
        const auto code = cell->replace(objects[source], objects[target]);
        assert(code == Code::OK);
        // deactivate the original object that has now been moved
        objects[source].deactivate();
    }

public:
    GarbageCollector(T * t, const int size)
        : objects(t), array_size(size)
    {
        assert(objects != nullptr);
    }

    int compact() {
        live.clear();
        dead.clear();

        for (auto i = 0; i < array_size; i++) {
            if (objects[i].active())
                live.push_back(i);
            else
                dead.push_back(i);
        }

        if (live.size() == 0 || dead.size() == 0)
            return 0;

        std::reverse(live.begin(), live.end());
        const auto size = std::min(live.size(), dead.size());
        for (auto i = 0; i < size; i++) {
            if (live[i] < dead[i])
                break;

            move(live[i], dead[i]);
        }
        return live.size();
    }

    inline int count() const {
        return live.size();
    }
};

template<class T>
using GC = GarbageCollector<T>;

}

#endif

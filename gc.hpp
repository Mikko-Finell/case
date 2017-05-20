#ifndef CASE_GC
#define CASE_GC

#include <cassert>
#include <vector>
#include <algorithm>

#include "code.hpp"

namespace CASE {

template<class T>
class GarbageCollector {

    const int array_size;
    std::vector<int> live;
    std::vector<int> dead;

    inline void move(T * objects, const int source, const int target) {
        assert(target < source);
        assert(target >= 0);
        assert(target < array_size);
        assert(source > 0);
        assert(source < array_size);
        assert(objects + target < objects + array_size - 1);
        assert(objects + source < objects + array_size);

        auto target_cell = objects[target].cell;
        auto source_cell = objects[source].cell;
        //assert(target_cell != nullptr);
        assert(source_cell != nullptr);

        const auto z = objects[source].z;
        source_cell->extract(z);

        objects[target] = objects[source];
        source_cell->insert(objects[target]);

        // deactivate the original object that has now been moved
        objects[source].deactivate();
    }

public:
    GarbageCollector(const int size) : array_size(size)
    {
    }

    int compact(T * objects) {
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

            move(objects, live[i], dead[i]);
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

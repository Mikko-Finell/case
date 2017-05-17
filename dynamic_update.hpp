#ifndef CASE_DYNUPD
#define CASE_DYNUPD

#include <vector>
#include <algorithm>
#include <random>

namespace CASE {
namespace update{

template<class T>
class Dynamic {
    std::mt19937 rng;
    std::vector<int> indices;
    T * objects;

public:
    Dynamic(T * obj) : objects(obj) {
        std::random_device rd;
        rng.seed(rd());
    }

    template<class World>
    void launch(World & world, const int subset) {
        indices.resize(world.count());
        std::iota(indices.begin(), indices.end(), 0);
        std::shuffle(indices.begin(), indices.end(), rng);
        if (subset < world.count()) {
            auto iterator = indices.begin();
            std::advance(iterator, subset);
            indices.erase(iterator, indices.end());
        }
        for (const auto i : indices)
            objects[i].update(world);
    }
};

} // update
} // CASE

#endif // DYNUPD

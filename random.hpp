#ifndef CASE_RAND
#define CASE_RAND

#include <array>
#include <algorithm>
#include <random>

namespace CASE {

class Random {
    mutable std::mt19937 engine;

public:

    Random(const int seed) {
        engine.seed(seed);
    }

    Random() {
#ifdef CASE_NORANDOM
        const auto seed = 0;
#else
        const auto seed = std::random_device()();
#endif
        engine.seed(seed);
    }

    void seed(const int n) {
        engine.seed(n);
    }

    int operator()(const int low, const int high) const {
        std::uniform_int_distribution<int> dist(low, high);
        return dist(engine);
    }

    bool boolean() const {
        return this->operator()(0, 1);
    }

    template<int low, int high>
    std::array<int, high - low> range() {
        std::array<int, high - low> array;
        std::iota(array.begin(), array.end(), low);
        std::shuffle(array.begin(), array.end(), engine);
        return array;
    }
};

template<int LOW, int HIGH>
class RDist {
    mutable std::mt19937 engine;
    mutable std::uniform_int_distribution<int> dist{LOW, HIGH};

public:
    RDist() {
#ifdef CASE_NORANDOM
        const auto seed = 0;
#else
        const auto seed = std::random_device()();
#endif
        engine.seed(seed);
    }

    int operator()() const {
        return dist(engine);
    }
};

using RBool = RDist<0, 1>;

} // CASE

#endif // RAND

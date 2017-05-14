#ifndef CASE_RAND
#define CASE_RAND

#include <random>

namespace CASE {

class Random {
    mutable std::mt19937 rng;

public:
    Random(const int seed) {
        rng.seed(seed);
    }

    Random() {
        const auto seed = std::random_device()();
        rng.seed(seed);
    }

    void seed(const int n) {
        rng.seed(n);
    }

    int operator()(const int low, const int high) const {
        std::uniform_int_distribution<int> dist(low, high);
        return dist(rng);
    }
};

} // CASE

#endif // RAND

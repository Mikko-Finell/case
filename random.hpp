#ifndef CASE_RAND
#define CASE_RAND

#include <algorithm>
#include <random>

namespace CASE {

#ifdef CASE_RAND_BEST
using Engine = std::mt19937;

#else
using Engine = std::minstd_rand;
//using Engine = std::ranlux24;
#endif

auto seed() {
#ifdef CASE_DETERMINISTIC
    return 0;
#else
    std::random_device rd;
    return rd();
#endif
}

template <class DistributionType>
class Random {
    mutable Engine engine{seed()};

public:
    int operator()(const int a, const int b) const {
        DistributionType dist(a, b);
        return dist(engine);
    }

    template <class Container>
    Container & shuffle(Container & container) {
        std::shuffle(std::begin(container), std::end(container), engine);
        return container;
    }

    template <class Container>
    auto shuffled(Container container) {
        std::shuffle(std::begin(container), std::end(container), engine);
        return container;
    }
};

using Uniform = Random<std::uniform_int_distribution<int>>;
using Gaussian = Random<std::normal_distribution<>>;
using Cauchy = Random<std::cauchy_distribution<>>;

template<int LOW, int HIGH>
class RDist {
    mutable Engine engine{seed()};
    mutable std::uniform_int_distribution<int> dist{LOW, HIGH};

public:
    int operator()() const {
        return dist(engine);
    }
};

template<int A, int B>
class RGaus {
    mutable Engine engine{seed()};
    mutable std::normal_distribution<> dist{A, B};

public:
    int operator()() const {
        return dist(engine);
    }
};

template<int A, int B>
class RCauc {
    mutable Engine engine{seed()};
    mutable std::cauchy_distribution<> dist{A, B};

public:
    int operator()() const {
        return dist(engine);
    }
};

using RBool = RDist<0, 1>;

} // CASE

#endif // RAND

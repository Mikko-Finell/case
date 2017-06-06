/* Author: Mikko Finell
 * License: Public Domain */

#ifndef CASE_RAND
#define CASE_RAND

#include <limits>
#include <algorithm>
#include <random>

namespace CASE {

#ifdef CASE_RAND_BEST
using Engine = std::mt19937;

#else
using Engine = std::minstd_rand;
//using Engine = std::ranlux24;
#endif

inline auto seed() {
#ifdef CASE_DETERMINISTIC
    static int index = 0;
    static const unsigned int seed[] = {
        170077028, 4157006078, 3702102293, 2899679562,
        2279478864, 1429673373, 3938844402, 2349274950
    };
    return static_cast<Engine::result_type>(seed[index++ % 8]);
#else
    static std::random_device rd;
    return rd();
#endif
}

template <class DistType, int A = std::numeric_limits<int>::min(),
                          int B = std::numeric_limits<int>::max()>
class Random {
    mutable Engine engine{seed()};
    mutable DistType dist{A, B};

public:
    void param(const int a, const int b) const {
        dist = DistType{a, b};
    }

    int operator()() const {
        return dist(engine);
    }

    int operator()(const int a, const int b) const {
        dist = DistType{a, b};
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

template <int A = 0, int B = 100>
using Uniform = Random<std::uniform_int_distribution<int>, A, B>;

template <int A = 0, int B = 100>
using Gaussian = Random<std::normal_distribution<>, A, B>;

template <int A = 0, int B = 100>
using Cauchy = Random<std::cauchy_distribution<>, A, B>;

using SBool = Uniform<0, 1>;

} // CASE

#endif // RAND

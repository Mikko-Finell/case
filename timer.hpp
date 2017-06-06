/* Author: Mikko Finell
 * License: Public Domain */

#ifndef CASE_TIMER
#define CASE_TIMER

#include <chrono>
#include <cmath>

namespace CASE {

using namespace std::chrono;
using namespace std::literals::chrono_literals;

class Timer {
    using clock = std::chrono::high_resolution_clock;
    clock::time_point start_time;
    double _duration = 0.0;

public:
    inline void start() {
        start_time = clock::now();
    }

    Timer() {
        start();
    }

    inline double reset() {
        _duration = dt();
        start_time = clock::now();
        return _duration;
    }

    inline double stop() {
        _duration += dt();
        return _duration;
    }

    inline double dt() const {
        const auto precision = 1000;
        const auto us = duration_cast<microseconds>(clock::now() - start_time);
        return std::round(us.count() / 1000.0 * precision) / precision;
    }

    inline double duration() const {
        return _duration;
    }
};

}

#endif

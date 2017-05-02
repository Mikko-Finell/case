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
    double duration = 0.0;

public:
    void start() {
        start_time = clock::now();
    }
    double stop() {
        const auto precision = 1000;
        const auto us = duration_cast<microseconds>(clock::now() - start_time);
        duration = std::round(us.count() / 1000.0 * precision) / precision;
        return dt();
    }

    inline double dt() const {
        return duration;
    }
};

}

#endif

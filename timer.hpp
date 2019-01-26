/* Author: Mikko Finell
 * License: Public Domain */

#ifndef CASE_TIMER
#define CASE_TIMER

#include <chrono>
#include <cmath>
#include <iostream>
#include <string>

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
        const auto us = duration_cast<microseconds>(clock::now() - start_time);
        return us.count() / 1000.0;
    }

    inline double duration() const {
        return _duration;
    }
};

class ScopeTimer {
    Timer timer;
    const std::string message;

public:
    ScopeTimer(const std::string & msg) : message(msg) {
    }

    ~ScopeTimer() {
        std::cout << message << ": " << timer.stop() << "ms\n";
    }
};

} // CASE

#endif

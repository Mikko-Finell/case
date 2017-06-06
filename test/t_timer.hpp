#include <algorithm>
#include <string>
#include <future>
#include <vector>
#include <random>
#include <cpptest.hpp>
#include "../timer.hpp"

namespace t_timer {

bool no_start() {
    CASE::Timer timer;
    return timer.dt() == 0;
}

bool no_stop() {
    CASE::Timer timer;
    timer.start();
    return timer.dt() == 0;
}

bool sleep_fzz() {
    using namespace std::chrono;
    const auto seed = std::random_device()();
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> dist(1, 1000);
    std::vector<std::future<bool>> futures;
    std::vector<bool> results;
    {
        cpptest::Module fuzz{"sleep fuzzing"};
        for (int i = 0; i < 10000; i++) {
            const auto t = dist(rng);
            futures.push_back(std::async(std::launch::async,
                [t]{
                    CASE::Timer timer;
                    timer.start();
                    std::this_thread::sleep_for(milliseconds(t));
                    const auto dt = timer.reset();
                    const auto accuracy = t < 10 ? 0.1 : 0.01;
                    return t * (1.0 - accuracy) < dt
                           && t * (1.0 + accuracy) > dt;
                }));
            const std::string name = "Sleep " + std::to_string(t) + " ms";
            fuzz.fn(name, [&futures, &results, i]{
                results.push_back(futures[i].get());
                return results[i];
            });
        }
    }
    return std::all_of(results.cbegin(), results.cend(), [](bool b){return b;});
}

bool startstop() {
    using namespace std::chrono;
    CASE::Timer t;

    t.start();
    std::this_thread::sleep_for(milliseconds(100)); // should count this dt
    t.stop();

    std::this_thread::sleep_for(milliseconds(100)); // should ignore this dt

    t.start();
    std::this_thread::sleep_for(milliseconds(100)); // should count this dt
    t.stop();

    std::this_thread::sleep_for(milliseconds(100)); // should ignore this dt

    t.start();
    std::this_thread::sleep_for(milliseconds(100)); // should count this dt
    t.stop();

    std::this_thread::sleep_for(milliseconds(100)); // should ignore this dt

    return t.duration() > 290 && t.duration() < 310;
}

void run() {
    cpptest::Module test{"timer"};
    test.fn("no start", no_start);
    test.fn("no stop", no_stop);
    test.fn("sleep fuzzing", sleep_fzz);
    test.fn("start, stop", startstop);
}

} // t_timer

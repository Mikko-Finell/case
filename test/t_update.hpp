#include <algorithm>
#include <cpptest.hpp>
#include "../update.hpp"

namespace t_update {

class Agent {
public:
    Agent update(Agent&) {
        Agent next;
        next.updated = true;
        return next;
    }
    bool updated = false;
};

bool serial_01() {
    constexpr auto SIZE = 100;
    CASE::update::Serial<Agent> upd;
    Agent current[SIZE];
    Agent next[SIZE];
    upd.launch(current, next, SIZE);
    return std::all_of(std::begin(next), std::end(next),
        [](const Agent & a){ return a.updated; });
}

bool serial_02() {
    constexpr auto SIZE = 100;
    CASE::update::Serial<Agent> upd;
    Agent current[SIZE];
    Agent next[SIZE];

    const auto generations = 1000;
    for (auto i = 0; i < generations; i++) {
        for (auto & agent : next)
            agent.updated = false;

        upd.launch(current, next, SIZE);
        upd.wait();
    }
    return std::all_of(std::begin(next), std::end(next),
        [](const Agent & a){ return a.updated; });
}

bool parallel_01() {
    constexpr auto SIZE = 100;
    CASE::update::Parallel<Agent> upd;
    Agent current[SIZE];
    Agent next[SIZE];
    upd.init();
    upd.wait();
    upd.launch(current, next, SIZE);
    upd.wait();
    upd.terminate();
    return std::all_of(std::begin(next), std::end(next),
        [](const Agent & a){ return a.updated; });
}

bool parallel_02() {
    constexpr auto SIZE = 100;
    CASE::update::Parallel<Agent> upd;
    Agent current[SIZE];
    Agent next[SIZE];
    upd.init();

    const auto generations = 1000;
    for (auto i = 0; i < generations; i++) {
        for (auto & agent : next)
            agent.updated = false;

        upd.launch(current, next, SIZE);
        upd.wait();
    }
    upd.terminate();
    return std::all_of(std::begin(next), std::end(next),
        [](const Agent & a){ return a.updated; });
}

void run() {
    cpptest::Module test{"update"};
    test.fn("serial update 01", serial_01);
    test.fn("serial update 02", serial_02);
    test.fn("parallel update 01", parallel_01);
    test.fn("parallel update 02", parallel_02);
}
}

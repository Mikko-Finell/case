#include <algorithm>
#include <cpptest.hpp>
#include "../updatemanager.hpp"

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
    CASE::update::Serial<Agent> upd;
#define SIZE 100
    Agent current[SIZE];
    Agent next[SIZE];
    upd.launch(current, next, SIZE);
#undef SIZE
    return std::all_of(std::begin(next), std::end(next),
        [](const Agent & a){ return a.updated; });
}

bool parallel_01() {
    CASE::update::Parallel<Agent> upd;
#define SIZE 100
    Agent current[SIZE];
    Agent next[SIZE];
    upd.init();
    upd.wait();
    upd.launch(current, next, SIZE);
    upd.wait();
    upd.terminate();
#undef SIZE
    return std::all_of(std::begin(next), std::end(next),
        [](const Agent & a){ return a.updated; });
}

void run() {
    cpptest::Module test{"update"};
    test.fn("serial update 01", serial_01);
    test.fn("parallel update 01", parallel_01);
}
}

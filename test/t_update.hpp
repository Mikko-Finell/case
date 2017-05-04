#include <future>
#include <algorithm>
#include <random>
#include <cpptest.hpp>
#include "../update.hpp"
#include "../array_buffer.hpp"

namespace t_update {

using namespace std::chrono;

class t1_Agent {
    int sleep = 0;
public:
    bool updated = false;
    int age = 0;
    t1_Agent(int ms = 0, int a = 0) : sleep(ms), age(a) {}
    t1_Agent update(t1_Agent&) {
        t1_Agent next{sleep, this->age + 1};
        next.updated = true;
        std::this_thread::sleep_for(milliseconds(sleep));
        return next;
    }
};

class t2_Agent {
public:
    int sleep = 0;
    bool updated = false;
    int age = 0;
    t2_Agent(int ms = 0, int a = 0) : sleep(ms), age(a) {}
    std::function<t2_Agent(t2_Agent&)> update;
};

bool serial_01() {
    constexpr auto SIZE = 100;
    CASE::update::Serial<t1_Agent> upd;
    t1_Agent current[SIZE];
    t1_Agent next[SIZE];
    upd.launch(current, next, SIZE);
    return std::all_of(std::begin(next), std::end(next),
        [](const t1_Agent & a){ return a.updated; });
}

bool serial_02() {
    constexpr auto SIZE = 100;
    CASE::update::Serial<t1_Agent> upd;
    t1_Agent current[SIZE];
    t1_Agent next[SIZE];

    const auto generations = 1000;
    for (auto i = 0; i < generations; i++) {
        for (auto & agent : next)
            agent.updated = false;

        upd.launch(current, next, SIZE);
        upd.wait();
    }
    return std::all_of(std::begin(next), std::end(next),
        [](const t1_Agent & a){ return a.updated; });
}

bool parallel_01() {
    constexpr auto SIZE = 100;
    CASE::update::Parallel<t1_Agent> upd;
    t1_Agent current[SIZE];
    t1_Agent next[SIZE];
    upd.init();
    upd.wait();
    upd.launch(current, next, SIZE);
    upd.wait();
    upd.terminate();
    return std::all_of(std::begin(next), std::end(next),
        [](const t1_Agent & a){ return a.updated; });
}

bool parallel_02() {
    constexpr auto SIZE = 100;
    CASE::update::Parallel<t1_Agent> upd;
    t1_Agent current[SIZE];
    t1_Agent next[SIZE];
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
        [](const t1_Agent & a){ return a.updated; });
}

bool manager_01() {
    constexpr auto SIZE = 100;
    CASE::update::Manager<t1_Agent> updm;
    t1_Agent current[SIZE];
    t1_Agent next[SIZE];

    updm.wait();
    updm.launch(current, next, SIZE).wait();

    return std::all_of(std::begin(next), std::end(next),
        [](const t1_Agent & a){ return a.updated; });
}

bool manager_hi() {
    constexpr auto SIZE = 100;
    CASE::update::Manager<t1_Agent> updm{CASE::Trigger{100, 0.1}};
    t1_Agent current[SIZE];
    t1_Agent next[SIZE];
    for (auto agent : current)
        agent = t1_Agent{10};
    updm.wait();
    updm.launch(current, next, SIZE).wait();

    return std::all_of(std::begin(next), std::end(next),
        [](const t1_Agent & a){ return a.updated; });
}

bool manager_lo_hi() {
    auto update = [] (t2_Agent& ag) {
        std::this_thread::sleep_for(milliseconds(ag.sleep));
        t2_Agent next = ag;
        next.sleep = ag.sleep + 1;
        next.age = ag.age + 1;
        next.updated = true;
        return next;
    };
    constexpr auto SIZE = 10;
    const auto generations = 10;
    t2_Agent current[SIZE];
    for (auto & a : current)
        a.update = update;
    t2_Agent next[SIZE];

    CASE::update::Manager<t2_Agent> updm{CASE::Trigger{50, 0.1}};
    CASE::ArrayBuffer<t2_Agent> arb{current, next};

    for (int i = 0; i < generations; i++) {
        updm.launch(arb.current(), arb.next(), SIZE).wait();
        arb.flip();
    }

    return std::all_of(std::begin(next), std::end(next),
        [](const t2_Agent & a){ return a.updated; })
    && std::all_of(std::begin(next), std::end(next),
        [generations](const t2_Agent & a){ return a.age == generations - 1; });
}

bool manager_hi_lo() {
    auto update = [] (t2_Agent& ag) {
        std::this_thread::sleep_for(milliseconds(ag.sleep));
        t2_Agent next = ag;
        next.sleep = std::max(ag.sleep - 1, 0);
        next.age = ag.age + 1;
        next.updated = true;
        return next;
    };
    constexpr auto SIZE = 10;
    const auto generations = 10;
    t2_Agent current[SIZE], next[SIZE];
    for (auto & a : current) {
        a.update = update;
        a.sleep = 10;
    }

    CASE::update::Manager<t2_Agent> updm{CASE::Trigger{50, 0.1}};
    CASE::ArrayBuffer<t2_Agent> arb{current, next};

    for (int i = 0; i < generations; i++) {
        updm.launch(arb.current(), arb.next(), SIZE).wait();
        arb.flip();
    }

    return std::all_of(std::begin(next), std::end(next),
        [](const t2_Agent & a){ return a.updated; })
    && std::all_of(std::begin(next), std::end(next),
        [generations](const t2_Agent & a){ return a.age == generations - 1; });
}

bool manager_rand_sleep() {
    const auto seed = std::random_device()();
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> dist(0, 2);

    auto update = [] (t2_Agent& ag) {
        const auto seed = std::random_device()();
        std::mt19937 rng(seed);
        std::uniform_int_distribution<int> dist(0, 2);

        std::this_thread::sleep_for(milliseconds(dist(rng)));
        t2_Agent next = ag;
        next.age = ag.age + 1;
        next.updated = true;
        return next;
    };
    constexpr auto SIZE = 10;
    t2_Agent current[SIZE], next[SIZE];
    for (auto & a : current)
        a.update = update;

    CASE::update::Manager<t2_Agent> updm{CASE::Trigger{10, 0.1}};
    CASE::ArrayBuffer<t2_Agent> arb{current, next};

    const auto generations = 100;
    for (int i = 0; i < generations; i++) {
        updm.launch(arb.current(), arb.next(), SIZE).wait();
        arb.flip();
    }

    return std::all_of(std::begin(next), std::end(next),
        [](const t2_Agent & a){ return a.updated; })
    && std::all_of(std::begin(next), std::end(next),
        [generations](const t2_Agent & a){ return a.age == generations - 1; });
}

bool manager_rand_trig() {
    const auto seed = std::random_device()();
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> dist(0, 30);

    auto update = [] (t2_Agent& ag) {
        std::this_thread::sleep_for(milliseconds(2));
        t2_Agent next = ag;
        next.age = ag.age + 1;
        next.updated = true;
        return next;
    };
    constexpr auto SIZE = 10;
    t2_Agent current[SIZE], next[SIZE];
    for (auto & a : current)
        a.update = update;

    CASE::update::Manager<t2_Agent> updm{CASE::Trigger{10, 0.1}};
    CASE::ArrayBuffer<t2_Agent> arb{current, next};

    const auto generations = 100;
    for (int i = 0; i < generations; i++) {
        updm.launch(arb.current(), arb.next(), SIZE).wait();
        arb.flip();
        if (dist(rng) < 15) {
            const auto midline = double(dist(rng));
            const auto tolerance = double(dist(rng)) / 10;
            updm.set_trigger(CASE::Trigger{midline, tolerance});
        }
    }

    return std::all_of(std::begin(next), std::end(next),
        [](const t2_Agent & a){ return a.updated; })
    && std::all_of(std::begin(next), std::end(next),
        [generations](const t2_Agent & a){ return a.age == generations - 1; });
}

bool manager_rand_both() {
    const auto seed = std::random_device()();
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> dist(0, 30);

    auto update = [] (t2_Agent& ag) {
        const auto seed = std::random_device()();
        std::mt19937 rng(seed);
        std::uniform_int_distribution<int> dist(0, 2);

        std::this_thread::sleep_for(milliseconds(dist(rng)));
        t2_Agent next = ag;
        next.age = ag.age + 1;
        next.updated = true;
        return next;
    };
    constexpr auto SIZE = 10;
    t2_Agent current[SIZE], next[SIZE];
    for (auto & a : current)
        a.update = update;

    CASE::update::Manager<t2_Agent> updm{CASE::Trigger{10, 0.1}};
    CASE::ArrayBuffer<t2_Agent> arb{current, next};

    const auto generations = 100;
    for (int i = 0; i < generations; i++) {
        updm.launch(arb.current(), arb.next(), SIZE).wait();
        arb.flip();
        if (dist(rng) < 15) {
            const auto midline = double(dist(rng));
            const auto tolerance = double(dist(rng)) / 10;
            updm.set_trigger(CASE::Trigger{midline, tolerance});
        }
    }

    return std::all_of(std::begin(next), std::end(next),
        [](const t2_Agent & a){ return a.updated; })
    && std::all_of(std::begin(next), std::end(next),
        [generations](const t2_Agent & a){ return a.age == generations - 1; });
}

void run() {
    auto module = new cpptest::Module{"update"};
    auto & test = *module;

    test.fn("serial update 01", serial_01);
    test.fn("serial update 02", serial_02);
    test.fn("parallel update 01", parallel_01);
    test.fn("parallel update 02", parallel_02);
    test.fn("manager update 01", manager_01);
    test.fn("manager trig hi", manager_hi);
    test.fn("manager trig lo->hi", manager_lo_hi);
    test.fn("manager trig hi->lo", manager_hi_lo);
    
    std::list<std::future<bool>> r0;
    for (int i = 0; i < 10; i++) {
        r0.push_back(std::async(std::launch::async,
            [](){ return manager_rand_sleep(); }
        ));
        auto & f = r0.back();
        test.fn("manager random sleep", [&f]{ return f.get(); });
    }

    std::list<std::future<bool>> r1;
    for (int i = 0; i < 10; i++) {
        r1.push_back(std::async(std::launch::async,
            [](){ return manager_rand_trig(); }
        ));
        auto & f = r1.back();
        test.fn("manager random trigger", [&f]{ return f.get(); });
    }

    std::list<std::future<bool>> r2;
    for (int i = 0; i < 10; i++) {
        r2.push_back(std::async(std::launch::async,
            [](){ return manager_rand_both(); }
        ));
        auto & f = r2.back();
        test.fn("manager random both", [&f]{ return f.get(); });
    }

    delete module;
}

}
#include <future>
#include <algorithm>
#include <random>
#include <cpptest.hpp>
#include "../update.hpp"
#include "../array_buffer.hpp"

namespace t_update {

using namespace std::chrono;

class t1_Agent {
public:
    int sleep = 0;
    bool updated = false;
    int age = 0;
    t1_Agent(int ms = 0, int a = 0) : sleep(ms), age(a) {}
    void update(t1_Agent & next) {
        next.age = age + 1;
        next.updated = true;
        std::this_thread::sleep_for(milliseconds(sleep));
    }
};

class t2_Agent {
public:
    int sleep = 0;
    bool updated = false;
    int age = 0;
    t2_Agent(int ms = 0, int a = 0) : sleep(ms), age(a) {}
    std::function<void(t2_Agent&)> update;
};

bool parallel_01() {
    constexpr auto SIZE = 100;
    CASE::update::Static<t1_Agent> upd;
    t1_Agent current[SIZE];
    t1_Agent next[SIZE];
    upd.wait();
    upd.launch(current, next, SIZE);
    upd.wait();
    upd.terminate();
    return std::all_of(std::begin(next), std::end(next),
        [](const t1_Agent & a){ return a.updated; });
}

bool parallel_02() {
    constexpr auto SIZE = 100;
    constexpr auto SUBSET = 50;
    CASE::update::Static<t1_Agent> upd;
    t1_Agent current[SIZE];
    t1_Agent next[SIZE];
    upd.wait();

    std::vector<bool> tests;
    auto highest_age = 0;
    const auto generations = 100;
    for (auto i = 0; i < generations; i++) {
        upd.launch(current, next, SIZE, SUBSET);
        upd.wait();

        auto count = 0;
        bool was_raised = false;
        for (auto & agent : next) {
            if (agent.updated)
                count++;
            agent.updated = false;
            if (agent.age > highest_age)
                was_raised = true;
        }
        tests.push_back(was_raised);
        tests.push_back(count == SUBSET);
    }
    upd.terminate();
    return std::all_of(tests.begin(), tests.end(),
        [](const bool b){ return b; });
}

bool manager_01() {
    constexpr auto SIZE = 100;
    CASE::update::Static<t1_Agent> updm;
    t1_Agent current[SIZE];
    t1_Agent next[SIZE];

    updm.wait();
    updm.launch(current, next, SIZE).wait();

    return std::all_of(std::begin(next), std::end(next),
        [](const t1_Agent & a){ return a.updated; });
}

bool manager_hi() {
    constexpr auto SIZE = 100;
    CASE::update::Static<t1_Agent> updm;
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
    auto update = [] (t2_Agent & next) {
        std::this_thread::sleep_for(milliseconds(next.sleep));
        next.sleep++;
        next.age++;
        next.updated = true;
    };
    constexpr auto SIZE = 10;
    const auto generations = 10;
    t2_Agent current[SIZE];
    for (auto & a : current)
        a.update = update;
    t2_Agent next[SIZE];

    CASE::update::Static<t2_Agent> updm;
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
    auto update = [] (t2_Agent & next) {
        std::this_thread::sleep_for(milliseconds(next.sleep));
        next.sleep = std::max(next.sleep - 1, 0);
        next.age++;
        next.updated = true;
    };
    constexpr auto SIZE = 10;
    const auto generations = 10;
    t2_Agent current[SIZE], next[SIZE];
    for (auto & a : current) {
        a.update = update;
        a.sleep = 10;
    }

    CASE::update::Static<t2_Agent> updm; 
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

    auto update = [] (t2_Agent & next) {
        const auto seed = std::random_device()();
        std::mt19937 rng(seed);
        std::uniform_int_distribution<int> dist(0, 2);

        std::this_thread::sleep_for(milliseconds(dist(rng)));
        next.age = next.age + 1;
        next.updated = true;
    };
    constexpr auto SIZE = 10;
    t2_Agent current[SIZE], next[SIZE];
    for (auto & a : current)
        a.update = update;

    CASE::update::Static<t2_Agent> updm;
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

    auto update = [] (t2_Agent & next) {
        std::this_thread::sleep_for(milliseconds(2));
        next.age = next.age + 1;
        next.updated = true;
    };
    constexpr auto SIZE = 10;
    t2_Agent current[SIZE], next[SIZE];
    for (auto & a : current)
        a.update = update;

    CASE::update::Static<t2_Agent> updm;
    CASE::ArrayBuffer<t2_Agent> arb{current, next};

    const auto generations = 100;
    for (int i = 0; i < generations; i++) {
        updm.launch(arb.current(), arb.next(), SIZE).wait();
        arb.flip();
        if (dist(rng) < 15) {
            const auto midline = double(dist(rng));
            const auto tolerance = double(dist(rng)) / 10;
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

    auto update = [] (t2_Agent & next) {
        const auto seed = std::random_device()();
        std::mt19937 rng(seed);
        std::uniform_int_distribution<int> dist(0, 2);

        std::this_thread::sleep_for(milliseconds(dist(rng)));
        next.age++;
        next.updated = true;
        return next;
    };
    constexpr auto SIZE = 10;
    t2_Agent current[SIZE], next[SIZE];
    for (auto & a : current)
        a.update = update;

    CASE::update::Static<t2_Agent> updm;
    CASE::ArrayBuffer<t2_Agent> arb{current, next};

    const auto generations = 100;
    for (int i = 0; i < generations; i++) {
        updm.launch(arb.current(), arb.next(), SIZE).wait();
        arb.flip();
        if (dist(rng) < 15) {
            const auto midline = double(dist(rng));
            const auto tolerance = double(dist(rng)) / 10;
        }
    }

    return std::all_of(std::begin(next), std::end(next),
        [](const t2_Agent & a){ return a.updated; })
    && std::all_of(std::begin(next), std::end(next),
        [generations](const t2_Agent & a){ return a.age == generations - 1; });
}

class t3_agent {
public:
    bool updated = false;
    void update(t3_agent & next) {
        next.updated = true;
    }
};

bool subset_parallel(int size, int subset, int gens) {
    CASE::update::Static<t3_agent> upd;
    upd.wait();

    auto ptr1 = new t3_agent[size];
    auto ptr2 = new t3_agent[size];
    CASE::ArrayBuffer<t3_agent> arb{ptr1, ptr2};

    std::vector<bool> results;
    for (int i = 0; i < gens; i++) {
        upd.launch(arb.current(), arb.next(), size, subset);
        upd.wait();
        arb.flip();

        int count = 0;
        for (auto i = 0; i < size; i++) {
            if (arb[i].updated)
                count++;
            arb[i].updated = false;
        }
        results.push_back(count == std::min(size, subset));
    }

    upd.terminate();

    delete [] ptr1;
    delete [] ptr2;

    return std::all_of(results.begin(), results.end(), [](bool b){ return b; });
}

bool subset_manager() {
    constexpr auto SIZE = 10;

    t3_agent current[SIZE], next[SIZE];
    for (auto & a : current) assert(a.updated == false);
    for (auto & a : next) assert(a.updated == false);

    CASE::update::Static<t3_agent> updm;
    CASE::ArrayBuffer<t3_agent> arb{current, next};

    updm.launch(current, next, SIZE, SIZE / 2).wait();
    
    int count = 0;
    for (const auto & agent : next) {
        if (agent.updated)
            count++;
    }
    return count == SIZE / 2;
}

bool copied() {
    constexpr auto SIZE = 3;
    constexpr auto SUBSET = 2;

    t1_Agent current[SIZE], next[SIZE];
    for (auto & a : current) {
        assert(a.updated == false);
        a.sleep = 10;
    }
    for (auto & a : next) {
        assert(a.updated == false);
    }

    CASE::update::Static<t1_Agent> updm;

    updm.launch(current, next, SIZE, SUBSET).wait();
    auto count = 0;
    for (auto & a : next)
        if (a.updated)
            count++;
    assert(count == SUBSET);

    std::vector<int> updated_indices;
    for (auto i = 0; i < SIZE; i++)
        if (next[i].updated)
            updated_indices.push_back(i);
    
    for (auto & a : current) a.updated = false;
    for (auto & a : next) a.updated = false;

    updm.launch(next, current, SIZE, SUBSET).wait();

    count = 0;
    for (auto & a : current) {
        if (a.updated)
            count++;
    }
    assert(count == SUBSET);

    auto t0 = 0;
    for (auto & a : current)
        if (a.age >= 1)
            t0++;
    
    return std::any_of(std::begin(current), std::end(current),
        [](auto&& a){ return a.age == 2; });
}

void run() {
    auto module = new cpptest::Module{"update"};
    auto & test = *module;

    test.fn("subset manager", subset_manager);
    test.fn("subset old are copied over", copied);

    const auto seed = std::random_device()();
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> pop(0, 100);
    std::uniform_int_distribution<int> subset(0, 100);

    for (auto i = 0; i < 100; i++) {
        const auto x = pop(rng), y = subset(rng);
        const auto name = std::string{"subset parallel, pop = "}
            + std::to_string(x) + " subset = " + std::to_string(y);
        test.fn(name, [x, y]{ return subset_parallel(x, y, 5); });
    }

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

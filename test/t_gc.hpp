#include <algorithm>
#include <future>
#include <vector>
#include <random>
#include <cpptest.hpp>
#include "../gc.hpp"

namespace t_gc {

class Agent {
    bool alive = false;

public:
    Agent (*update)(Agent &);
    bool active() const { return alive; }
    void activate() { alive = true; }
    void deactivate() { alive = false; }
};

template<class Agent>
bool _test(Agent * a, Agent * b, const int size) {
    CASE::GC<Agent> gc{a, b, size};
    gc.copy_and_compact();

    std::vector<int> live, dead;
    for (auto i = 0; i < size; i++) {
        if (a[i].active())
            live.push_back(i);
        else
            dead.push_back(i);
    }
    return std::all_of(live.begin(), live.end(),
        [&](auto&& live_index){
            return std::none_of(dead.begin(), dead.end(),
                [live_index](auto&& dead_index)
                { return dead_index < live_index; }); })
    && gc.count() == live.size();
}

bool gc_01() {
    const auto n = 20;
    Agent a[n], b[n];
    for (auto i = 0; i < n; i++) {
        if (i >= n / 2) {
            a[i].activate();
            b[i].activate();
        }
    }
    return _test<Agent>(a, b, n);
}

bool gc_02() {
    const auto n = 1000;
    Agent a[n], b[n];
    for (auto i = 0; i < n; i++) {
        if (i % 3 == 0 || i % 5 == 0 || i % 7 == 0) {
            a[i].activate();
            b[i].activate();
        }
    }
    return _test(a, b, n);
}

bool gc_all_live() {
    constexpr auto n = 100;
    Agent a[n], b[n];
    for (int i = 0; i < n; i++) {
        a[i].activate();
        b[i].activate();
    }

    return _test(a, b, n);
}

bool gc_all_dead() {
    const auto n = 100;
    Agent a[n], b[n];
    for (auto i = 0; i < n; i++)
        a[n].deactivate();

    return _test(a, b, n);
}

bool fuzz() {
    const auto seed = std::random_device()();
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> pop(2, 1000);
    std::uniform_int_distribution<int> gen(2, 200);

    std::vector<std::future<bool>> futures;
    for (int i = 0; i < 10; i++) {
        const auto test_gens = gen(rng);
        const auto test_pop = pop(rng);

        futures.push_back(std::async(std::launch::async,
            [test_pop, test_gens]{
                const auto seed = std::random_device()();
                std::mt19937 rng(seed);
                std::uniform_int_distribution<int> random_bool(0, 1);
                std::vector<Agent> a, b;
                for (auto i = 0; i < test_pop; i++) {
                    a.emplace_back();
                    b.emplace_back();
                }
                std::vector<bool> _res;
                for (auto i = 0; i < test_gens; i++) {
                    for (auto k = 0; k < test_pop; k++) {
                        if (random_bool(rng))
                            a[k].activate();
                        else
                            a[k].deactivate();
                    }
                    _res.push_back(_test(a.data(), b.data(), test_pop));
                }
                return std::all_of(_res.cbegin(), _res.cend(),
                        [](bool b){return b;});
            }));
    }
    return std::all_of(futures.begin(), futures.end(),
            [](auto & future){return future.get();});
}

bool bug01() {
    const auto size = 6;
    std::vector<Agent> a, b;
    for (int i = 0; i < size; i++) {
        a.emplace_back();
        b.emplace_back();
        a.back().activate();
        b.back().activate();
    }
    a[0].deactivate();
    a[2].deactivate();
    return _test(a.data(), b.data(), size);
}

void run() {
    cpptest::Module test_gc{"garbage collector"};
    test_gc.fn("worst case", gc_01);
    test_gc.fn("semi random case", gc_02);
    test_gc.fn("all live", gc_all_live);
    test_gc.fn("all dead", gc_all_dead);
    test_gc.fn("fuzzing", fuzz);
    test_gc.fn("test bug 01", bug01);
}

} // t_gc

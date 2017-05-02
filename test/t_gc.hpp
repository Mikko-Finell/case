#include <algorithm>
#include <deque>
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

bool gc_01() {
    const auto n = 10;
    Agent a[n], b[n];
    for (auto i = 0; i < n; i++) {
        if (i >= n / 2) {
            a[i].activate();
            b[i].activate();
        }
    }

    CASE::GC<Agent> gc{a, b, n};
    gc.copy_and_compact();

    std::deque<int> live, dead;
    for (auto i = 0; i < n; i++) {
        if (a[i].active())
            live.push_back(i);
        else
            dead.push_back(i);
    }

    // for every index in alive, check that every index in dead is greater
    return std::all_of(live.begin(), live.end(),
        [&](auto&& live_index){
            return std::none_of(dead.begin(), dead.end(),
                [live_index](auto&& dead_index)
                { return dead_index < live_index; }); })
    && gc.count() == live.size();
}

bool gc_02() {
    const auto n = 10;
    Agent a[n], b[n];
    for (auto i = 0; i < n; i++) {
        if (i % 3 == 0 || i % 5 == 0 || i % 7 == 0) {
            a[i].activate();
            b[i].activate();
        }
    }
    
    CASE::GC<Agent> gc{a, b, n};
    gc.copy_and_compact();

    std::deque<int> live, dead;
    for (auto i = 0; i < n; i++) {
        if (a[i].active())
            live.push_back(i);
        else
            dead.push_back(i);
    }

    // for every index in alive, check that every index in dead is greater
    return std::all_of(live.begin(), live.end(),
        [&](auto&& live_index){
            return std::none_of(dead.begin(), dead.end(),
                [live_index](auto&& dead_index)
                { return dead_index < live_index; }); })
    && gc.count() == live.size();
}

bool gc_all_live() {
    constexpr auto n = 100;
    Agent a[n], b[n];
    int x = 0;
    for (int i = 0; i < n; i++) {
        ++x;
        a[i].activate();
        b[i].activate();
    }

    CASE::GC<Agent> gc{a, b, n};
    gc.copy_and_compact();

    std::deque<int> live, dead;
    for (auto i = 0; i < n; i++) {
        if (a[i].active())
            live.push_back(i);
        else
            dead.push_back(i);
    }

    return gc.count() == n;
}

bool gc_all_dead() {
    const auto n = 100;
    Agent a[n], b[n];
    for (auto i = 0; i < n; i++)
        a[n].deactivate();

    CASE::GC<Agent> gc{a, b, n};
    gc.copy_and_compact();

    std::deque<int> live, dead;
    for (auto i = 0; i < n; i++) {
        if (a[i].active())
            live.push_back(i);
        else
            dead.push_back(i);
    }
    return live.empty() && dead.size() == n && gc.count() == 0;
}

void run() {
    cpptest::Module test_gc{"garbage collector"};
    test_gc.fn("worst case", gc_01);
    test_gc.fn("random case", gc_02);
    test_gc.fn("all live", gc_all_live);
    test_gc.fn("all dead", gc_all_dead);
}

} // t_gc

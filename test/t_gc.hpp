#include <algorithm>
#include <future>
#include <vector>
#include <random>
#include <cpptest.hpp>
#include "../random.hpp"
#include "../index.hpp"
#include "../code.hpp"
#include "../cell.hpp"
#include "../gc.hpp"
#include "../grid.hpp"

namespace t_gc {

class Agent {
    bool alive = false;

public:
    int z = 0;
    Agent(int layer = 0) : z(layer) {}
    
    bool active() const { return alive; }
    void activate() { alive = true; }
    void deactivate() { alive = false; }
    CASE::ZCell<Agent, 1> * cell = nullptr;
};

template<class Agent>
bool _test(Agent * a, const int size) {
    CASE::GC<Agent> gc{size};
    std::vector<CASE::ZCell<Agent, 1>> cells{std::size_t(size)};
    for (auto i = 0; i < size; i++)
        cells[i].insert(a[i]);

    gc.compact(a);

    std::vector<int> live, dead;
    for (auto i = 0; i < size; i++) {
        if (a[i].active())
            live.push_back(i);
        else
            dead.push_back(i);
    }
    
    const bool t0 = std::all_of(live.begin(), live.end(),
        [&](auto&& live_index){
            return std::none_of(dead.begin(), dead.end(),
                [live_index](auto&& dead_index)
                { return dead_index < live_index; }); });
    const bool t1 = gc.count() == live.size();

    std::vector<bool> results;
    for (auto i = 0; i < size; i++) {
        if (a[i].active()) {
            auto & cell = a[i].cell;
            results.push_back(cell->get() == &a[i]);
            auto count = 0;
            for (auto & c : cells) {
                if (c.get() == &a[i]) {
                    count++;
                    results.push_back(&c == cell);
                }
            }
            results.push_back(count == 1);
        }
    }
    const bool t2 = std::all_of(results.begin(), results.end(),
                                [](bool b){return b;});
    return t0 && t1 && t2;

}

bool gc_01() {
    const auto n = 20;
    Agent a[n];
    for (auto i = 0; i < n; i++) {
        if (i >= n / 2)
            a[i].activate();
    }
    return _test<Agent>(a, n);
}

bool gc_02() {
    const auto n = 1000;
    Agent a[n];
    for (auto i = 0; i < n; i++) {
        if (i % 3 == 0 || i % 5 == 0 || i % 7 == 0)
            a[i].activate();
    }
    return _test(a, n);
}

bool gc_all_live() {
    constexpr auto n = 100;
    Agent a[n];
    for (int i = 0; i < n; i++)
        a[i].activate();

    return _test(a, n);
}

bool gc_all_dead() {
    const auto n = 100;
    Agent a[n];
    for (auto i = 0; i < n; i++)
        a[n].deactivate();

    return _test(a, n);
}

bool fuzz() {
    CASE::Random rand_int;

    std::vector<std::future<bool>> futures;
    for (int i = 0; i < 1; i++) {
        const auto test_gens = rand_int(2, 20);
        const auto test_pop = rand_int(2, 10);

        futures.push_back(std::async(std::launch::deferred,
            [test_pop, test_gens]{
                CASE::Random rng;
                std::vector<Agent> a;
                for (auto i = 0; i < test_pop; i++)
                    a.emplace_back();

                std::vector<bool> _res;
                for (auto i = 0; i < test_gens; i++) {
                    for (auto k = 0; k < test_pop; k++) {
                        a[k].cell = nullptr;
                        if (rng.boolean())
                            a[k].activate();
                        else
                            a[k].deactivate();
                    }
                    _res.push_back(_test(a.data(), test_pop));
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
    std::vector<Agent> a;
    for (int i = 0; i < size; i++) {
        a.emplace_back();
        a.back().activate();
    }
    a[0].deactivate();
    a[2].deactivate();
    return _test(a.data(), size);
}

bool it01() {
    using namespace CASE;
    // create agents
    constexpr auto cols = 10;
    constexpr auto rows = 10;
    
    auto agents = new Agent[cols * rows];

    // create grid
    // randomly deactivate some agents
    Random rng;
    Grid<ZCell<Agent, 1>> grid{cols, rows};
    for (auto y = 0; y < rows; y++) {
        for (auto x = 0; x < cols; x++) {
            grid(x, y).insert(agents[CASE::index(x, y, cols)]);
            if (rng(0, 10) < 5)
                grid(x, y).get()->deactivate();
        }
    }
    // gc compact
    GarbageCollector<Agent> gc{cols * rows};
    gc.compact(agents);

    std::vector<bool> results;
    
    for (auto y = 0; y < rows; y++) {
        for (auto x = 0; x < cols; x++) {
            auto agent = grid(x, y).get();
            results.push_back(
                agent->cell == &grid(x, y)
                && agent->cell->get() == agent
            );
        }
    }

    return std::all_of(results.begin(), results.end(), [](bool b){return b;});
}

void run() {
    cpptest::Module test_gc{"garbage collector"};
    test_gc.fn("worst case", gc_01);
    test_gc.fn("semi random case", gc_02);
    test_gc.fn("all live", gc_all_live);
    test_gc.fn("all dead", gc_all_dead);
    test_gc.fn("fuzzing", fuzz);
    //test_gc.fn("test bug 01", bug01);

    //cpptest::Module it{"integration test of cell and gc"};
    //it.fn("agent is reassinged when moved", it01);
}

} // t_gc

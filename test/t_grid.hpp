#include <future>
#include <string>
#include <algorithm>
#include <vector>
#include <cpptest.hpp>
#include "../array_buffer.hpp"
#include "../cell.hpp"
#include "../grid.hpp"
#include "../neighbors.hpp"
#include "../timer.hpp"

namespace t_grid {

bool wrap(int size, int index, int expected) {
    return CASE::wrap(index, size) == expected;
}

bool wrap_forward() {
    return wrap(4, 0, 0)
        && wrap(4, 1, 1)
        && wrap(4, 2, 2)
        && wrap(4, 3, 3)
        && wrap(4, 4, 0)
        && wrap(4, 5, 1)
        && wrap(4, 6, 2)
        && wrap(4, 7, 3)
        && wrap(4, 8, 0)
        && wrap(4, 9, 1)
        && wrap(4, 10, 2);
}

bool wrap_backward() {
    return wrap(4, 0, 0)
        && wrap(4, -1, 3)
        && wrap(4, -2, 2)
        && wrap(4, -3, 1)
        && wrap(4, -4, 0)
        && wrap(4, -5, 3)
        && wrap(4, -6, 2)
        && wrap(4, -7, 1)
        && wrap(4, -8, 0)
        && wrap(4, -9, 3)
        && wrap(4, -10, 2);
}

bool rm(int cols, int rows) {
    auto ptr = new int[cols * rows];
    for (auto i = 0; i < cols * rows; i++)
        ptr[i] = i;

    int i = 0;
    std::vector<bool> results;
    for (int y = 0; y < rows; y++) {
        for (int x = 0; x < cols; x++) {
            results.push_back(CASE::index(x, y, cols) == i);
            i++;
        }
    }
    delete [] ptr;
    return std::all_of(results.begin(), results.end(), [](bool b){ return b; });
}

struct Pos {
    int x=666, y=777;
    bool operator==(const Pos & other) const {
        return other.x == x && other.y == y;
    }
};

class Agent {
public:
    Pos pos;
    CASE::SimpleCell<Agent> * cell;
};

using Cell = CASE::SimpleCell<Agent>;

bool check(Cell & cell, const Pos & size) {
    std::vector<bool> results;
    const auto pos = cell.get()->pos;
    static const int range[3] = {-1, 0, 1};
    for (const auto y : range) {
        for (const auto x : range) {
            auto expected_pos = Pos{pos.x + x, pos.y + y};

            if (pos.x + x < 0)
                expected_pos.x = size.x - 1;
            if (pos.x + x == size.x)
                expected_pos.x = 0;

            if (pos.y + y < 0)
                expected_pos.y = size.y - 1;
            if (pos.y + y == size.y)
                expected_pos.y = 0;

            if (!(cell.neighbors(x, y)->pos == expected_pos)) {
                std::cout << "Error, expected " << expected_pos.x
                    << ", " << expected_pos.y << " but got "
                    << cell.neighbors(x, y)->pos.x << ", "
                    << cell.neighbors(x, y)->pos.y << std::endl;
            }
            results.push_back(cell.neighbors(x, y)->pos == expected_pos);
        }
    }
    return std::all_of(results.begin(), results.end(), [](bool b){ return b; });
}

bool t(int x, int y) {
    CASE::Grid<Cell> grid{x, y};
    auto agents = new Agent[x * y];

    for (auto row = 0; row < y; row++) {
        for (auto col = 0; col < x; col++) {
            grid(col, row).insert(agents[CASE::index(col, row, x)]);
            grid(col, row).get()->pos = Pos{col, row};
        }
    }

    std::vector<bool> results;
    for (auto row = 0; row < y; row++) {
        for (auto col = 0; col < x; col++)
            results.push_back(check(grid(col, row), Pos{x, y}));
    }

    delete [] agents;
    return std::all_of(results.begin(), results.end(), [](bool b){ return b; });
}

void run() {
    cpptest::Module twrap{"wrap"};
    twrap.fn("wrap forwards", wrap_forward);
    twrap.fn("wrap backwards", wrap_backward);

    cpptest::Module tindex{"row major index"};
    for (auto x = 1; x < 100; x++) {
        const auto name = std::to_string(x) + "x1";
        tindex.fn(name, [x]{ return rm(x, 1); });
    }
    for (auto y = 1; y < 100; y++) {
        const auto name = std::string("1x") + std::to_string(y);
        tindex.fn(name, [y]{ return rm(1, y); });
    }
    for (auto k = 1; k < 100; k++) {
        const auto name = std::to_string(k) + "x" + std::to_string(k);
        tindex.fn(name, [k]{ return rm(k, k); });
    }

    const auto seed = std::random_device()();
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> dist(1, 1000);

    for (auto x = 1; x < 100; x++) {
        const auto y = dist(rng);
        const auto name = std::to_string(x) + "x" + std::to_string(y);
        tindex.fn(name, [x, y]{ return rm(x, y); });
    }
    for (auto y = 1; y < 100; y++) {
        const auto x = dist(rng);
        const auto name = std::to_string(x) + "x" + std::to_string(y);
        tindex.fn(name, [x, y]{ return rm(x, y); });
    }
    for (auto i = 1; i < 100; i++) {
        const auto x = dist(rng), y = dist(rng);
        const auto name = std::to_string(x) + "x" + std::to_string(y);
        tindex.fn(name, [x, y]{ return rm(x, y); });
    }

    std::vector<std::future<bool>> results;
    {
        std::uniform_int_distribution<int> dist(1, 120);
        for (auto k = 0; k < 1000; k++) {
            const auto _seed = seed * k;
            results.push_back(std::async(std::launch::async, [_seed, &dist]{
                std::vector<bool> r;
                std::mt19937 rng(_seed);
                for (auto i = 0; i < 5; i++) {
                    const auto x = dist(rng), y = dist(rng);
                    r.push_back(t(x, y));
                }
                return std::all_of(r.begin(), r.end(), [](bool b){ return b; });
            }));
        }
    }
    {
        cpptest::Module test{"grid"};
        for (auto & result : results)
            test.fn("connectome", [&]{ return result.get(); });

        test.fn("1 x 1", []{ return t(1, 1); });
        test.fn("2 x 2", []{ return t(2, 2); });
        test.fn("3 x 3", []{ return t(3, 3); });
        test.fn("4 x 4", []{ return t(4, 4); });
        test.fn("5 x 5", []{ return t(5, 5); });
        test.fn("6 x 6", []{ return t(6, 6); });
        test.fn("7 x 7", []{ return t(7, 7); });
        test.fn("8 x 8", []{ return t(8, 8); });

        test.fn("2 x 1", []{ return t(1+1, 1); });
        test.fn("3 x 1", []{ return t(1+2, 1); });
        test.fn("4 x 1", []{ return t(1+3, 1); });
        test.fn("5 x 1", []{ return t(1+4, 1); });
        test.fn("6 x 1", []{ return t(1+5, 1); });
        test.fn("7 x 1", []{ return t(1+6, 1); });
        
        test.fn("2 x 3", []{ return t(2, 2+1); });
        test.fn("3 x 4", []{ return t(3, 3+1); });
        test.fn("4 x 5", []{ return t(4, 4+1); });
        test.fn("5 x 6", []{ return t(5, 5+1); });
        test.fn("6 x 7", []{ return t(6, 6+1); });
        test.fn("7 x 8", []{ return t(7, 7+1); });
        test.fn("8 x 9", []{ return t(8, 8+1); });
        test.fn("2 x 1", []{ return t(1+1, 1); });
        test.fn("3 x 2", []{ return t(2+1, 2); });
        test.fn("4 x 3", []{ return t(3+1, 3); });
        test.fn("5 x 4", []{ return t(4+1, 4); });
        test.fn("6 x 5", []{ return t(5+1, 5); });
        test.fn("7 x 6", []{ return t(6+1, 6); });
    }
}

}

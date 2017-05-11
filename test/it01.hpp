#include <cpptest.hpp>
#include "../array_buffer.hpp"
#include "../update.hpp"
#include "../grid.hpp"
#include "../cell.hpp"

namespace it01 {

class Agent0 {

public:
    int index = 0;

    Agent0 update(Agent0&a) {
        return a;
    }

    bool operator==(const Agent0& other) const {
        return this == &other;
    }
};

bool direct_3x3() {
    const auto cols = 3, rows = 3;
    const auto size = (cols * rows);
    static int range[3] = {-1,0,1};

    Agent0 all[size];
    for (auto i = 0; i < size; i++) 
        all[i].index = i;
    auto & self = all[4];
    auto neighbors = CASE::neighbors::Direct<Agent0>{&self, cols, rows};
    bool bs[9];
    int i = 0;
    for (const auto y : range) {
        for (const auto x : range) {
            bs[i] = neighbors(x, y) == all[i];
            i++;
        }
    }

    return std::all_of(std::begin(bs), std::end(bs), [](bool b){return b;});
}

bool direct_3x3_top_left() {
    const auto cols = 3, rows = 3;
    const auto size = (cols * rows);
    static int range[3] = {-1,0,1};

    Agent0 all[size];
    for (auto i = 0; i < size; i++) 
        all[i].index = i;
    auto & self = all[0];
    auto access = CASE::neighbors::Direct<Agent0>{&self, cols, rows};

    return access(-1, -1).index == 8
        && access(0, -1).index == 6
        && access(1, -1).index == 7
        && access(-1, 0).index == 2
        && access(0, 0).index == 0
        && access(1, 0).index == 1
        && access(-1, 1).index == 5 
        && access(0, 1).index == 3
        && access(1, 1).index == 4;
}

bool direct_3x3_top_right() {
    const auto cols = 3, rows = 3;
    const auto size = (cols * rows);
    static int range[3] = {-1,0,1};

    Agent0 all[size];
    for (auto i = 0; i < size; i++) 
        all[i].index = i;
    auto & self = all[2];
    auto access = CASE::neighbors::Direct<Agent0>{&self, cols, rows};

    return access(-1, -1).index == 7
        && access(0, -1).index == 8
        && access(1, -1).index == 6
        && access(-1, 0).index == 1
        && access(0, 0).index == 2
        && access(1, 0).index == 0
        && access(-1, 1).index == 4 
        && access(0, 1).index == 5
        && access(1, 1).index == 3;
}

bool direct_3x3_bottom_right() {
    const auto cols = 3, rows = 3;
    const auto size = (cols * rows);
    static int range[3] = {-1,0,1};

    Agent0 all[size];
    for (auto i = 0; i < size; i++) 
        all[i].index = i;
    auto & self = all[8];
    auto access = CASE::neighbors::Direct<Agent0>{&self, cols, rows};

    return access(-1, -1).index == 4
        && access(0, -1).index == 5
        && access(1, -1).index == 3
        && access(-1, 0).index == 7
        && access(0, 0).index == 8
        && access(1, 0).index == 6
        && access(-1, 1).index == 1 
        && access(0, 1).index == 2
        && access(1, 1).index == 0;
}

bool direct_3x3_bottom_left() {
    const auto cols = 3, rows = 3;
    const auto size = (cols * rows);
    static int range[3] = {-1,0,1};

    Agent0 all[size];
    for (auto i = 0; i < size; i++) 
        all[i].index = i;
    auto & self = all[6];
    auto access = CASE::neighbors::Direct<Agent0>{&self, cols, rows};

    return access(-1, -1).index == 5
        && access(0, -1).index == 3
        && access(1, -1).index == 4
        && access(-1, 0).index == 8
        && access(0, 0).index == 6
        && access(1, 0).index == 7
        && access(-1, 1).index == 2 
        && access(0, 1).index == 0
        && access(1, 1).index == 1;
}

void run() {
    cpptest::Module t2{"buffered world"};
    t2.fn("direct access", direct_3x3);
    t2.fn("top left", direct_3x3_top_left);
    t2.fn("top right", direct_3x3_top_right);
    t2.fn("bottom right", direct_3x3_bottom_right);
    t2.fn("bottom left", direct_3x3_bottom_left);
}

}

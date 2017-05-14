#include "../random.hpp"
#include "../neighbors.hpp"
#include "../quad.hpp"
#include "../grid.hpp"
#include "../vm.hpp"

#define COLUMNS 100
#define ROWS 200
#define CELL_SIZE 5

class Wolfram {
    int x, y;

    bool parent_is_active() const {
        auto neighbors = CASE::neighbors::Direct<Wolfram>{this, COLUMNS, ROWS};
        return neighbors(0, -1).active;
    }

public:
    int age = 0;
    bool live = false;
    bool active = false;
    int index = 0;

    Wolfram(int _x = 0, int _y = 0) : x(CELL_SIZE * _x), y(CELL_SIZE * _y)
    {
    }

    void update(Wolfram & next) const {
        auto neighbors = CASE::neighbors::Direct<Wolfram>{this, COLUMNS, ROWS};

        int pattern = 0b000;
        if (neighbors(-1, -1).live)
            pattern = pattern | 0b100;
        if (neighbors(0, -1).live)
            pattern = pattern | 0b010;
        if (neighbors(1, -1).live)
            pattern = pattern | 0b001;

        static const int r30 [8] = { 0, 1, 1, 1, 1, 0, 0, 0 };
        static const int r90 [8] = { 0, 1, 0, 1, 1, 0, 1, 0 };
        static const int r105[8] = { 1, 0, 0, 1, 0, 1, 1, 0 };
        static const int r106[8] = { 0, 1, 0, 1, 0, 1, 1, 0 };
        static const int r110[8] = { 0, 1, 1, 1, 0, 1, 1, 0 };
        static const int r126[8] = { 0, 1, 1, 1, 1, 1, 1, 0 };
        static const int r150[8] = { 0, 1, 1, 0, 1, 0, 0, 1 };

        static const int rules[7][8] = {
            { 0, 1, 1, 1, 1, 1, 1, 0 }, // 126
            { 0, 1, 0, 1, 1, 0, 1, 0 }, // 90
            { 0, 1, 1, 1, 1, 0, 0, 0 }, // 30 
            { 1, 0, 0, 1, 0, 1, 1, 0 }, // 105
            { 0, 1, 0, 1, 0, 1, 1, 0 }, // 106
            { 0, 1, 1, 1, 0, 1, 1, 0 }, // 110
            { 0, 1, 1, 0, 1, 0, 0, 1 }  // 150
        };

        if (active) {
            next.active = false;
            next.age++;

            if (rules[next.age % 7][pattern])
                next.live = true;
            else
                next.live = false;
        }
        else if (parent_is_active())
            next.active = true;
    }

    void draw(sf::Vertex * vs) const {
        if (live)
            CASE::quad(x, y, CELL_SIZE, CELL_SIZE,
                    0, 0, 0, vs);

        else if (active)
            CASE::quad(x, y, CELL_SIZE, CELL_SIZE,
                    255, 255, 100, vs);

        else
            CASE::quad(x, y, CELL_SIZE, CELL_SIZE,
                    255, 255, 255, vs);
    }
};

struct Wolframs_Rules {
    using Agent         = Wolfram;
    using Update        = CASE::Update<Wolfram>;
    using Graphics      = CASE::Graphics<Wolfram>;

    const int columns = COLUMNS;
    const int cell_size = CELL_SIZE;
    const int rows = ROWS;
    double framerate = 100.0;
    const int subset = columns * rows;
    const char* title = "Wolfram rules";
    const sf::Color bgcolor = sf::Color{220, 220, 220};

    void init(Wolfram * agents) {
        int index = 0;
        for (auto y = 0; y < ROWS; y++) {
            for (auto x = 0; x < COLUMNS; x++) {
                auto & agent = agents[CASE::index(x, y, COLUMNS)];
                agent = Wolfram{x, y};
                agent.index = index++;
            }
        }
        CASE::Random rng;
        for (auto x = 0; x < COLUMNS; x++) {
            auto & agent = agents[CASE::index(x, 0, COLUMNS)];
            //if (rng(1, 100) < 50)
                //agent.live = true;
        }
        for (auto x = 0; x < COLUMNS; x++) {
            auto & agent = agents[CASE::index(x, 1, COLUMNS)];
            agent.active = true;
        }

        auto & agent = agents[CASE::index(COLUMNS / 2, 0, COLUMNS)];
        agent.live = true;
    };
};

int main() {
    CASE::vm::Totalistic<Wolframs_Rules>();
}

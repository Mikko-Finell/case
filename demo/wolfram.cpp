#include "../random.hpp"
#include "../neighbors.hpp"
#include "../quad.hpp"
#include "../grid.hpp"
#include "../static_sim.hpp"
#include "../random.hpp"

#define COLUMNS 300
#define ROWS 300
#define CELL_SIZE 2

int RULESET = 0;

class Wolfram {
    int x, y;

public:
    bool live = false;
    bool scanline = false;
    int index = 0;

    Wolfram(int _x = 0, int _y = 0) : x(CELL_SIZE * _x), y(CELL_SIZE * _y)
    {
    }

    void update(Wolfram & next) const {
        auto neighbors = CASE::CAdjacent<Wolfram>{this, COLUMNS, ROWS};
        int pattern = 0b000;
        if (neighbors(-1, -1).live)
            pattern = pattern | 0b100;
        if (neighbors(0, -1).live)
            pattern = pattern | 0b010;
        if (neighbors(1, -1).live)
            pattern = pattern | 0b001;
        static const int rules[7][8] = {
            { 0, 1, 1, 1, 1, 0, 0, 0 },
            { 0, 1, 0, 1, 1, 0, 1, 0 },
            { 1, 0, 0, 1, 0, 1, 1, 0 },
            { 0, 1, 0, 1, 0, 1, 1, 0 },
            { 0, 1, 1, 1, 0, 1, 1, 0 },
            { 0, 1, 1, 1, 1, 1, 1, 0 },
            { 0, 1, 1, 0, 1, 0, 0, 1 }
        };
        if (scanline) {
            next.scanline = false;
            next.live = rules[RULESET][pattern];
        }
        else if (neighbors(0, -1).scanline)
            next.scanline = true;
    }

    void draw(sf::Vertex * vs) const {
        if (live)
            CASE::quad(x, y, CELL_SIZE, CELL_SIZE, 0, 0, 0, vs);
        else if (scanline)
            CASE::quad(x, y, CELL_SIZE, CELL_SIZE, 255, 255, 100, vs);
        else
            CASE::quad(x, y, CELL_SIZE, CELL_SIZE, 255, 255, 255, vs);
    }
};

struct Wolframs_Rules {
    using Agent = Wolfram;
    const int columns = COLUMNS;
    const int cell_size = CELL_SIZE;
    const int rows = ROWS;
    double framerate = 100.0;
    const int subset = columns * rows;
    const char* title = "Wolfram rules";
    const sf::Color bgcolor = sf::Color{220, 220, 220};

    void init(Wolfram * agents) {
        static CASE::Uniform<0, 10> rand;
        int index = 0;
        for (auto y = 0; y < ROWS; y++) {
            for (auto x = 0; x < COLUMNS; x++) {
                auto & agent = agents[CASE::index(x, y, COLUMNS)];
                agent = Wolfram{x, y};
                agent.index = index++;
            }
        }
        for (auto x = 0; x < COLUMNS; x++) {
            auto & agent = agents[CASE::index(x, 1, COLUMNS)];
            agent.scanline = true;
        }

        for (auto x = 0; x < COLUMNS; x++) {
            auto & agent = agents[CASE::index(x, 0, COLUMNS)];
            if (rand() < 3)
                agent.live = true;
        }
    };

    void postprocessing(Wolfram * agents) {
        static bool pressed = false;
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::PageDown)) {
            if (!pressed) {
                RULESET = (RULESET + 1) % 7;
                std::cout << RULESET << std::endl;
                pressed = true;
            }
        }
        else pressed = false;
    }
};

int main() {
    CASE::Static<Wolframs_Rules>();
}

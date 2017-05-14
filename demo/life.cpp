#include "../random.hpp"
#include "../neighbors.hpp"
#include "../quad.hpp"
#include "../grid.hpp"
#include "../vm.hpp"

#define COLUMNS 80
#define ROWS 60
#define CELL_SIZE 10

class Life {
    int x, y, age = 255;

    int popcount() const {
        auto neighbors = CASE::neighbors::Direct<Life>{this, COLUMNS, ROWS};
        static const int range[3] = {-1, 0, 1};
        auto count = 0;
        for (const auto y : range) {
            for (const auto x : range) {
                if (x == 0 && y == 0)
                    continue;
                if (neighbors(x, y).live)
                    count++;
            }
        }
        return count;
    }

public:
    bool live = false;
    int index = 0;

    Life(int _x = 0, int _y = 0) : x(CELL_SIZE * _x), y(CELL_SIZE * _y)
    {
    }

    void update(Life & next) const {
        const auto count = popcount();
        if (live) {
            if (count < 2)
                next.live = false;

            if (count > 3)
                next.live = false;
        }
        else {
            if (count == 3) {
                next.live = true;
                next.age = 0;
            }
        }
        next.age++;
    }

    void draw(sf::Vertex * vs) const {
        /*
        if (live)
            CASE::quad(x+1, y+1, CELL_SIZE-2, CELL_SIZE-2,
                    0, 0, 0, vs);
        else
            CASE::quad(x, y, CELL_SIZE, CELL_SIZE,
                    255, 255, 255, vs);
        */
        auto clamp = [](auto x){
            return x > 255 ? 255 : x < 0 ? 0 : x;
        };
        if (live)
            CASE::quad(x+1, y+1, CELL_SIZE-2, CELL_SIZE-2,
                    255-clamp(age), 0, 0, vs);
        else
            CASE::quad(x, y, CELL_SIZE, CELL_SIZE,
                    255-clamp(age), 255-clamp(age), 255-clamp(age), vs);
    }
};

struct GameOfLife {
    using Agent         = Life;
    using Update        = CASE::Update<Life>;
    using Graphics      = CASE::Graphics<Life>;

    const int columns = COLUMNS;
    const int cell_size = CELL_SIZE;
    const int rows = ROWS;
    double framerate = 20.0;
    const int subset = columns * rows;
    const char* title = "GameOfLife";
    const sf::Color bgcolor = sf::Color::Black;

    void init(Life * agents) {
        CASE::Random rng;
        int index = 0;
        for (auto y = 0; y < ROWS; y++) {
            for (auto x = 0; x < COLUMNS; x++) {
                auto & agent = agents[CASE::index(x, y, COLUMNS)];
                agent = Life{x, y};
                if (rng(1, 100) < 25)
                    agent.live = true;
                agent.index = index++;
            }
        }
    };
};

int main() {
    CASE::vm::Totalistic<GameOfLife>();
}

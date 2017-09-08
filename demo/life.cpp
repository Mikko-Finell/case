#include "../random.hpp"
#include "../neighbors.hpp"
#include "../quad.hpp"
#include "../grid.hpp"
#include "../static_sim.hpp"

#define COLUMNS 512
#define ROWS 512
#define CELL_SIZE 2

class Life {
    int x, y;

public:
    bool live = false;
    int index = 0;

    Life(int _x = 0, int _y = 0) : x(CELL_SIZE * _x), y(CELL_SIZE * _y)
    {
    }

    void update(Life & next) const {
        auto neighbors = CASE::CAdjacent<Life>{this};
        static const int range[3] = {-1, 0, 1};
        auto count = 0;
        for (const auto y : range) {
            for (const auto x : range) {
                if (x == 0 && y == 0) continue;
                if (neighbors(x, y).live) count++;
            }
        }
        if (live && (count < 2 || count > 3)) next.live = false;
        else if (count == 3) next.live = true;
    }

    void draw(sf::Vertex * vs) const {
        static constexpr int pad = 0;
        if (live)
            CASE::quad(x, y, CELL_SIZE-pad, CELL_SIZE-pad, 0, 0, 0, vs);
        else
            CASE::quad(x, y, CELL_SIZE-pad, CELL_SIZE-pad, 255, 255, 255, vs);
    }
};

struct GameOfLife {
    using Agent = Life;
    static constexpr int columns = COLUMNS;
    static constexpr int rows = ROWS;
    static constexpr int cell_size = CELL_SIZE;
    static constexpr int subset = columns * rows;
    double framerate = 60.0;
    const char* title = "Conways Life";
    const sf::Color bgcolor = sf::Color::White;

    void init(Life * agents) {
        int index = 0;
        for (auto y = 0; y < ROWS; y++) {
            for (auto x = 0; x < COLUMNS; x++) {
                auto & agent = agents[CASE::index(x, y, COLUMNS)];
                agent = Life{x, y};
                agent.index = index++;
            }
        }
        const auto cx = COLUMNS/2;
        const auto cy = ROWS/2;
        const auto distance = (COLUMNS+ROWS)/4;
        static CASE::Uniform<0, 100> dist;
        for (auto y = 0; y < columns; y++) {
            for (auto x = 0; x < rows; x++) {
                auto & agent = agents[CASE::index(x, y, columns)];
                const auto dx = abs(cx-x), dy = abs(cy-y);
                const auto d = sqrt(dx*dx + dy*dy);
                if (d <= distance && (dist() > 50))
                    agent.live = true;
            }
        }
    }

    void postprocessing(Agent *) {}
};

int main() {
    CASE::Static<GameOfLife>();
}

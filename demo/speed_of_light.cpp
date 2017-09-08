#include "../random.hpp"
#include "../neighbors.hpp"
#include "../quad.hpp"
#include "../grid.hpp"
#include "../static_sim.hpp"

#define COLUMNS 21
#define ROWS 21
#define CELL_SIZE 15

class Light {
    int x, y;

public:
    bool live = false;
    int index = 0;

    Light(int _x = 0, int _y = 0) : x(CELL_SIZE * _x), y(CELL_SIZE * _y)
    {
    }

    void update(Light & next) const {
        auto neighbors = CASE::CAdjacent<Light>{this};
        next.live = live || neighbors(-1, 0).live || neighbors(0, -1).live
                    || neighbors(1, 0).live || neighbors(0, 1).live;
    }

    void draw(sf::Vertex * vs) const {
        static constexpr int pad = 1;
        if (live)
            CASE::quad(x, y, CELL_SIZE-pad, CELL_SIZE-pad,
                    0, 0, 0, vs);
        else
            CASE::quad(x, y, CELL_SIZE-pad, CELL_SIZE-pad,
                    255, 255, 255, vs);
    }
};

struct SpeedOfLight {
    using Agent = Light;

    static constexpr int columns = COLUMNS;
    static constexpr int rows = ROWS;
    static constexpr int cell_size = CELL_SIZE;
    double framerate = 1.0;
    const char* title = "Speed of Light";
    const sf::Color bgcolor = sf::Color::White;

    void init(Light * agents) {
        int index = 0;
        for (auto y = 0; y < ROWS; y++) {
            for (auto x = 0; x < COLUMNS; x++) {
                auto & agent = agents[CASE::index(x, y, COLUMNS)];
                agent = Light{x, y};
                agent.index = index++;
            }
        }
        auto & agent = agents[CASE::index(COLUMNS/2, ROWS/2, COLUMNS)];
        agent.live = true;
    }

    void postprocessing(Agent *) {}
};

int main() {
    CASE::Static<SpeedOfLight>();
}

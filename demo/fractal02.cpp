
#include "../random.hpp"
#include "../neighbors.hpp"
#include "../quad.hpp"
#include "../grid.hpp"
#include "../static_sim.hpp"

#define COLUMNS 256
#define ROWS 256
#define CELL_SIZE 2

class Automaton {
    int x, y;

public:
    bool live = false;
    int index = 0;

    Automaton(int _x = 0, int _y = 0) : x(CELL_SIZE * _x), y(CELL_SIZE * _y)
    {
    }

    void update(Automaton & next) const {
        if (live == false) {
            auto neighbors = CASE::CAdjacent<Automaton>{this};
            //static const int dirs[4][2] = {
                //{0,-1}, {1,0}, {0,1}, {-1,0}
            //};
            static const int dirs[4][2] = {
                {-1,-1}, {1,1}, {-1,1}, {1,-1}
            };
            auto count = 0;
            for (auto i = 0; i < 4; i++) {
                if (neighbors(dirs[i][0],dirs[i][1]).live)
                    count++;
            }
            if (count == 1 || count == 4)
                next.live = true;
        }

    }

    void draw(sf::Vertex * vs) const {
        static constexpr int pad = 0;
        if (live)
            CASE::quad(x, y, CELL_SIZE-pad, CELL_SIZE-pad,
                    0, 0, 0, vs);
        else
            CASE::quad(x, y, CELL_SIZE-pad, CELL_SIZE-pad,
                    255, 255, 255, vs);
    }
};

struct Fractal {
    using Agent = Automaton;

    static constexpr int columns = COLUMNS;
    static constexpr int rows = ROWS;
    static constexpr int cell_size = CELL_SIZE;
    static constexpr int subset = columns * rows;
    double framerate = 60.0;
    const char* title = "Game of Automaton";
    const sf::Color bgcolor = sf::Color::White;

    void init(Automaton * agents) {
        int index = 0;
        for (auto y = 0; y < ROWS; y++) {
            for (auto x = 0; x < COLUMNS; x++) {
                auto & agent = agents[CASE::index(x, y, COLUMNS)];
                agent = Automaton{x, y};
                agent.index = index++;
            }
        }
        auto & agent = agents[CASE::index(COLUMNS/2, ROWS/2, COLUMNS)];
        agent.live = true;
    }

    void postprocessing(Agent *) {}
};

int main() {
    CASE::Static<Fractal>();
}

#include "../random.hpp"
#include "../neighbors.hpp"
#include "../quad.hpp"
#include "../grid.hpp"
#include "../static_sim.hpp"

#define COLUMNS 512
#define ROWS 512
#define CELL_SIZE 2

struct Color {
    int index, commonness = 0;
    Color(int i) : index(i) {}
    bool operator<(const Color & other) const {
        return commonness < other.commonness;
    }
};

class Automata {
    int x, y;

public:
    int color_index;
    int index = 0;

    Automata(int _x = 0, int _y = 0) : x(CELL_SIZE * _x), y(CELL_SIZE * _y)
    {
    }

    void update(Automata & next) const {
        auto neighbors = CASE::CAdjacent<Automata>{this, COLUMNS, ROWS};
        Color colors[6] = {0,1,2,3,4,5};
        static const int range[3] = {-1, 0, 1};
        for (const auto Y : range) {
            for (const auto X : range) {
                if (X!=0 || Y!=0)
                    colors[neighbors(X, Y).color_index].commonness++;
            }
        }
        std::sort(std::begin(colors), std::end(colors));
        if (colors[4].commonness == colors[5].commonness) {
            static CASE::Uniform<4,5> rng;
            next.color_index = colors[rng()].index;
        }
        else
            next.color_index = colors[5].index;
    }

    void draw(sf::Vertex * vs) const {
        static constexpr int color[6][3] = {
            {255,0,0},{0,255,0},{0,0,255},{255,255,0},{255,0,255},{0,255,255}
        };
        CASE::quad(x, y, CELL_SIZE, CELL_SIZE,
                color[color_index][0],
                color[color_index][1],
                color[color_index][2],
                vs);
    }
};

struct ColorSwitchers {
    using Agent = Automata;
    const int columns = COLUMNS;
    const int cell_size = CELL_SIZE;
    const int rows = ROWS;
    double framerate = 60;
    const int subset = 0.1 * columns * rows;
    const char* title = "Color Switchers";
    const sf::Color bgcolor = sf::Color::Black;

    void init(Automata * agents) {
        CASE::Uniform<0,5> random;
        int index = 0;
        for (auto y = 0; y < ROWS; y++) {
            for (auto x = 0; x < COLUMNS; x++) {
                auto & agent = agents[CASE::index(x, y, COLUMNS)];
                agent = Automata{x, y};
                agent.index = index++;
                agent.color_index = random();
            }
        }
    }

    void postprocessing(Automata *) {}
};

int main() {
    CASE::Static<ColorSwitchers>();
}

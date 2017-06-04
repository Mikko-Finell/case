#include "../random.hpp"
#include "../neighbors.hpp"
#include "../quad.hpp"
#include "../grid.hpp"
#include "../static_sim.hpp"

#define COLUMNS 256
#define ROWS 256
#define CELL_SIZE 3

int rng(const int low, const int high) {
    static CASE::Uniform<> rand;
    return rand(low, high);
}

static constexpr int color[6][3] = {
    {255,0,0},   // red
    {0,255,0},   // green
    {0,0,255},   // blue
    {255,255,0}, // yellow
    {255,0,255}, // purple
    {0,255,255}, // cyan
};

struct Color {
    int index;
    int commonness = 0;
    Color(int i) : index(i) {}
    bool operator<(const Color & other) const {
        return commonness < other.commonness;
    }
};

class Automata {
    int x, y;
    int color_index;

public:
    int index = 0;

    Automata(int _x = 0, int _y = 0) : x(CELL_SIZE * _x), y(CELL_SIZE * _y)
    {
    }

    void update(Automata & next) const {
        auto neighbors = CASE::CMemAdjacent<Automata>{this, COLUMNS, ROWS};
        // count neighbors colors
        // set own to be same as majority
        Color colors[6] = {0,1,2,3,4,5};

        static const int range[3] = {-1, 0, 1};
        for (const auto Y : range) {
            for (const auto X : range) {
                if (X==0 && Y==0)
                    continue;
                const int i = neighbors(X, Y).getcolor();
                colors[i].commonness++;
            }
        }
        std::sort(std::begin(colors), std::end(colors));
        if (colors[4].commonness == colors[5].commonness)
            next.color_index = colors[rng(4,5)].index;
        else
            next.color_index = colors[5].index;
    }

    int getcolor() const {
        return color_index;
    }

    void setcolor(int i) {
        assert(i >= 0 && i < 6);
        color_index = i;
    }

    void draw(sf::Vertex * vs) const {
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

                agent.setcolor(random());
            }
        }
    }

    void preprocessing() {
    }
};

int main() {
    CASE::Static<ColorSwitchers>();
}

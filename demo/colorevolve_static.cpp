#include <iostream>

#include "../quad.hpp"
#include "../index.hpp"
#include "../random.hpp"
#include "../grid.hpp"
#include "../cell.hpp"
#include "../static_sim.hpp"

#define COLUMNS 512
#define ROWS 512
#define CELL_SIZE 2

int mfactor = 10;
int start_r = 255, start_g = 255, start_b = 255;

template <int MIN, int MAX>
int clamp(const int value) {
    return value < MIN ? MIN : value > MAX ? MAX : value;
}

class Bacteria {
    int x, y;
    int r = 255, g = 255, b = 255;
    bool alive = false;

public:
    int index = 0;

    Bacteria(int _x = 0, int _y = 0) : x(CELL_SIZE * _x), y(CELL_SIZE * _y)
    {
        deactivate();
    }
    void mutate(const Bacteria & parent) {
        static CASE::Uniform<> rand;
        r = clamp<0,255>(parent.r + rand(-mfactor, mfactor));
        g = clamp<0,255>(parent.g + rand());
        b = clamp<0,255>(parent.b + rand());
    }
    void setrgb(const int _r, const int _g, const int _b) {
        r = _r; g = _g; b = _b;
    }
    void draw(sf::Vertex * vs) const {
        CASE::quad(x, y, CELL_SIZE, CELL_SIZE, r,g,b, vs);
    }
    inline bool active() const { return alive; }
    inline void activate() { alive = true; }
    inline void deactivate() { alive = false; }
    void update(Bacteria & next) const;

    CASE::Uniform<0, 7> random;
};

void Bacteria::update(Bacteria & self) const {
    if (active())
        return;

    static const int dirs[][2] = {
        {-1,-1}, { 0,-1}, { 1,-1},
        {-1, 0},          { 1, 0},
        {-1, 1}, { 0, 1}, { 1, 1}
    };

    const auto i = self.random();
    const auto x = dirs[i][0], y = dirs[i][1];

    auto neighbors = CASE::CAdjacent<Bacteria>{this};
    auto & neighbor = neighbors(x, y);
    if (neighbor.active()) {
        self.mutate(neighbor);
        self.activate();
    }
}

struct Config {
    using Agent = Bacteria;

    static constexpr int columns = COLUMNS;
    static constexpr int rows = ROWS;
    static constexpr int subset = columns * rows;
    static constexpr int cell_size = CELL_SIZE;

    const double framerate = 15;
    const char* title = "Color Evolution";
    const sf::Color bgcolor{255, 255, 255};

    void init(Agent * agents) {
        CASE::Uniform<1, 100> rand;
        int index = 0;
        for (auto y = 0; y < ROWS; y++) {
            for (auto x = 0; x < COLUMNS; x++) {
                auto & agent = agents[CASE::index(x, y, COLUMNS)];
                agent = Bacteria{x, y};
                agent.index = index++;
            }
        }
        auto & agent = agents[CASE::index(columns/2, rows/2, COLUMNS)];

        start_r = rand(0, 255);
        start_g = rand();
        start_b = rand();

        agent.setrgb(start_r, start_g, start_b);
        agent.activate();
    }

    void postprocessing(Agent *) {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::PageUp)) {
            mfactor = clamp<0,100>(mfactor + 1);
            std::cout << "Mutation Factor  " << mfactor << std::endl;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::PageDown)) {
            mfactor = clamp<0,100>(mfactor - 1);
            std::cout << "Mutation Factor  " << mfactor << std::endl;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::B)) {
            start_r = 0; start_g = 0; start_b = 0;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
            start_r = 255; start_g = 255; start_b = 255;
        }
    }
};

int main() {
    CASE::Static<Config>();
}

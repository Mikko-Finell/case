#include <CASE/quad.hpp>
#include <CASE/index.hpp>
#include <CASE/random.hpp>
#include <CASE/grid.hpp>
#include <CASE/cell.hpp>
#include <CASE/helper.hpp>
#include <CASE/static_sim.hpp>

#define COLUMNS 512
#define ROWS 512
#define CELL_SIZE 2

int mfactor = 10, start_r = 255, start_g = 255, start_b = 255;

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
        r = CASE::clamp<0,255>(parent.r + rand(-mfactor, mfactor));
        g = CASE::clamp<0,255>(parent.g + rand());
        b = CASE::clamp<0,255>(parent.b + rand());
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
};

void Bacteria::update(Bacteria & next) const {
    if (active())
        return;
    auto neighbors = CASE::CAdjacent<Bacteria>{this, COLUMNS, ROWS};
    bool alone = true;
    for (auto y = -1; y <= 1; y++) {
        for (auto x = -1; x <= 1; x++) {
            if (x == 0 && y == 0)
                continue;
            auto & neighbor = neighbors(x, y);
            if (neighbor.active()) {
                static CASE::Uniform<-1, 1> uv;
                auto & neighbor = neighbors(uv(),uv());
                if (neighbor.active()) {
                    next.mutate(neighbor);
                    next.activate();
                }
                return;
            }
        }
    }
}

struct Config {
    using Agent = Bacteria;
    static constexpr int columns = COLUMNS;
    static constexpr int rows = ROWS;
    static constexpr int subset = columns * rows;
    static constexpr int cell_size = CELL_SIZE;
    const double framerate = 60;
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
        CASE::Uniform<0,255> color;
        agent.setrgb(start_r, start_g, start_b);
        agent.activate();
    }

    void postprocessing(Agent *) {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::PageUp)) {
            mfactor = CASE::clamp<0,100>(mfactor + 1);
            std::cout << "Mutation Factor  " << mfactor << std::endl;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::PageDown)) {
            mfactor = CASE::clamp<0,100>(mfactor - 1);
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

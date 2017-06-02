#include <iostream>

#include "../quad.hpp"
#include "../index.hpp"
#include "../random.hpp"
#include "../grid.hpp"
#include "../cell.hpp"
#include "../simulator.hpp"

#define COLUMNS 300
#define ROWS 300
#define CELL_SIZE 3

int mfactor = 5;

template <int MIN, int MAX>
int clamp(const int value) {
    return value < MIN ? MIN : value > MAX ? MAX : value;
}

class Bacteria {
    bool alive = false;
    int r = 0, g = 0, b = 0;

public:
    using Cell = CASE::ZCell<Bacteria, 2>;
    int z = 0;
    Cell * cell = nullptr;

    Bacteria();
    Bacteria(const Bacteria * parent);
    void update();
    void draw(const int, const int, std::vector<sf::Vertex> & vertices) const;
    bool active() const { return alive; }
    void activate() { alive = true;; }
    void deactivate() { alive = false;; }
};

Bacteria::Bacteria() {
    static CASE::RDist<0, 255> rcolor;
    r = rcolor(); g = rcolor(); b = rcolor();
}

Bacteria::Bacteria(const Bacteria * parent) {
    static CASE::Uniform rand;
    r = clamp<0,255>(parent->r + rand(-mfactor, mfactor));
    g = clamp<0,255>(parent->g + rand(-mfactor, mfactor));
    b = clamp<0,255>(parent->b + rand(-mfactor, mfactor));
}

void Bacteria::update() {
    static CASE::RDist<-1, 1> uv;
    auto neighbors = cell->neighbors();
    neighbors(uv(), uv()).spawn(Bacteria{this});
}

void Bacteria::draw(const int x, const int y, std::vector<sf::Vertex> & vs) const {
    const auto size = vs.size();
    vs.resize(size + 4);
    CASE::quad(x*CELL_SIZE, y*CELL_SIZE, CELL_SIZE, CELL_SIZE, r,g,b, &vs[size]);
}

struct Config {
    using Agent = Bacteria;
    using Cell = Agent::Cell;
    using Grid = CASE::Grid<Cell>;

    static constexpr int columns = COLUMNS;
    static constexpr int rows = ROWS;
    static constexpr int cell_size = CELL_SIZE;
    static constexpr int subset = columns * rows * Cell::depth;

    const double framerate = 60;
    const char* title = "Color Evolution";
    const sf::Color bgcolor{255, 255, 255};

    void init(Grid & grid, CASE::AgentManager<Agent> & manager) {
        grid.clear();
        manager.clear();
        grid(columns/2,rows/2).spawn(Agent{});
    }

    void postprocessing(Grid & /*grid*/) {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Insert)) {
            mfactor = clamp<0,100>(mfactor + 1);
            std::cout << "M Factor  " << mfactor << std::endl;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Delete)) {
            mfactor = clamp<0,100>(mfactor - 1);
            std::cout << "M Factor  " << mfactor << std::endl;
        }
    }
};

int main() {
    CASE::simulator::Dynamic<Config>();
}

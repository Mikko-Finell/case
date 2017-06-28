#include <iostream>

#include "../quad.hpp"
#include "../index.hpp"
#include "../random.hpp"
#include "../grid.hpp"
#include "../cell.hpp"
#include "../dynamic_sim.hpp"
#include "../helper.hpp"

#define COLUMNS 256
#define ROWS 256
#define CELL_SIZE 5
#define ANT_LAYER 0
#define PHEROMONE_LAYER 1
#define HP_MAX 25

enum Type { Ant, Pheromone };

struct Vector { int x=0, y=0; };

namespace Ants {
class Agent {
    bool alive = false;
    int hp = HP_MAX;
    Type type;
    int dir = 0;

public:
    using Cell = CASE::ZCell<Agent, 2>;

    int z = 0;
    Cell * cell = nullptr;

    Agent(const Type t = Type::Ant, const int layer = ANT_LAYER)
        : z(layer), type(t)
    {}

    void update();
    void draw(const int, const int, std::vector<sf::Vertex> & vertices) const;
    bool active() const { return alive; }
    void activate() { alive = true; }
    void deactivate() { alive = false; }
};

template <int OP>
int turn(const int dir) { return CASE::wrap<8>(dir + OP); }
int left(const int dir) { return turn<-1>(dir); }
int right(const int dir) { return turn<1>(dir); }

int steer(const int dir, bool bias_l, bool bias_c, bool bias_r) {
    constexpr int stdev = 5;
    constexpr int max = 100;
    constexpr int avg = max / 2;
    static CASE::Gaussian<avg, stdev> rand;

    int shift = 0;

    if (bias_l && bias_r && !bias_c)
        shift += max / 2;
    else if (!bias_c) {
        if (bias_l)
            shift -= max / 4;
        else if (bias_r)
            shift += max / 4;
    }

    const auto r = CASE::wrap<100>(rand() + shift);

    if (r < avg - stdev)
        return right(dir);

    else if (r > avg + stdev)
        return left(dir);

    else // go straight
        return dir;
}

Vector vector(const int dir) {
    switch (dir) {
        case 0:  return {-1, -1};
        case 1:  return { 0, -1};
        case 2:  return { 1, -1};

        case 3:  return { 1,  0};
        case 4:  return { 1,  1};
        case 5:  return { 0,  1};

        case 6:  return {-1,  1};
        default: return {-1,  0};
    }
}

void Agent::update() {
    if (type == Ant) {
        auto neighbors = cell->neighbors();

        auto beneath = neighbors(0, 0).getlayer(PHEROMONE_LAYER);
        if (beneath == nullptr)
            neighbors(0, 0).spawn(Agent{Type::Pheromone, PHEROMONE_LAYER});

        else
            beneath->hp = HP_MAX;

        auto v = vector(left(dir));
        auto pheromone = neighbors(v.x, v.y).getlayer(PHEROMONE_LAYER);
        const auto bias_l = pheromone == nullptr;

        v = vector(dir);
        pheromone = neighbors(v.x, v.y).getlayer(PHEROMONE_LAYER);
        const auto bias_c = pheromone == nullptr;

        v = vector(right(dir));
        pheromone = neighbors(v.x, v.y).getlayer(PHEROMONE_LAYER);
        const auto bias_r = pheromone == nullptr;

        dir = steer(dir, bias_l, bias_c, bias_r);
        const auto vec = vector(dir);
        neighbors(vec.x, vec.y).insert(this);
    }
    else {
        if (--hp < 0)
            deactivate();
    }
}

void Agent::draw(const int x, const int y, std::vector<sf::Vertex> & vs) const {
    const auto i = vs.size();
    vs.resize(i + 4);
    int r = 0, g = 0, b = 0, pad = 0;
    if (type == Pheromone) {
        ++pad;
        g = 200;
    }
    CASE::quad(x * CELL_SIZE + pad, y * CELL_SIZE + pad,
            CELL_SIZE - 2 * pad, CELL_SIZE - 2 * pad, r,g,b, &vs[i]);
}
} // Ants

struct Config {
    using Agent = Ants::Agent;
    using Cell = Agent::Cell;
    using Grid = CASE::Grid<Cell>;

    static constexpr int columns = COLUMNS;
    static constexpr int rows = ROWS;
    static constexpr int cell_size = CELL_SIZE;
    static constexpr int subset = 0.1 * columns * rows * Cell::depth;

    const double framerate = 60;
    const char* title = "Ants";
    const sf::Color bgcolor{255, 255, 255};

    void init(Grid & grid, CASE::AgentManager<Agent> & manager) {
        grid.clear();
        manager.clear();

        CASE::Uniform<0, columns - 1> x;
        CASE::Uniform<0, rows - 1> y;

        for (auto i = 0; i < 200; i++)
            grid(x(), y()).spawn(Agent{});
    }

    void postprocessing(Grid & /*grid*/) {
    }
};

int main() {
    CASE::Dynamic<Config>();
}

#include <cmath>

#include "../quad.hpp"
#include "../index.hpp"
#include "../random.hpp"
#include "../grid.hpp"
#include "../cell.hpp"
#include "../dynamic_sim.hpp"

#define COLUMNS 128
#define ROWS 128
#define CELL_SIZE 4
#define CELL_LAYER 1
#define ANT_LAYER 0

constexpr double pi() { return std::atan(1) * 4; }
enum Type { Ant, Cell };

namespace Langton {
class Agent {
    bool alive = false;
    int angle = 0; // degrees

public:
    int z = 0;
    using Cell = CASE::ZCell<Agent, 2>;
    Cell * cell = nullptr;

    Agent(Type type = Type::Cell);
    void update();
    void draw(const int, const int, std::vector<sf::Vertex> & vertices) const;
    bool active() const { return alive; }
    void activate() { alive = true; }
    void deactivate() { alive = false; }
};

Agent::Agent(const Type _type) {
    if (_type == Type::Cell)
        z = CELL_LAYER;
    else {
        static CASE::Uniform<0,3> rand;
        angle = 90 * rand();
        z = ANT_LAYER;
    }
}

void Agent::update() {
    if (z == ANT_LAYER) {
        auto neighbors = cell->neighbors();
        constexpr static int turn_amt = 90;
        auto cell = neighbors(0, 0).getlayer(CELL_LAYER);
        if (cell == nullptr) {
            neighbors(0, 0).spawn(Agent{Type::Cell});
            angle -= turn_amt;
        }
        else {
            cell->deactivate();
            angle += turn_amt;
        }
        neighbors(
            std::cos(angle * pi() / 180), std::sin(angle * pi() / 180)
        ).insert(this);
    }
}

void Agent::draw(const int x, const int y, std::vector<sf::Vertex> & vs) const {
    const auto i = vs.size();
    vs.resize(i + 4);
    int r = (z?0:200), g = 0, b = 0, pad = 1;
    CASE::quad(x * CELL_SIZE, y * CELL_SIZE,
            CELL_SIZE-pad, CELL_SIZE-pad, r,g,b, &vs[i]);
}
} // namespace Langton

struct Config {
    using Agent = Langton::Agent;
    using Cell = Agent::Cell;
    using Grid = CASE::Grid<Cell>;

    static constexpr int columns = COLUMNS;
    static constexpr int rows = ROWS;
    static constexpr int cell_size = CELL_SIZE;
    static constexpr int subset = columns * rows * Cell::depth;
    const double framerate = 60;
    const char* title = "Langtons Ant";
    const sf::Color bgcolor{255, 255, 255};

    void init(Grid & grid, CASE::AgentManager<Agent> & manager) {
        grid.clear();
        manager.clear();
        grid(COLUMNS/2, ROWS/2).spawn(Agent{Ant});
    }

    void postprocessing(Grid & /*grid*/) {}
};

int main() {
    CASE::Dynamic<Config>();
}

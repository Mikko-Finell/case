#include <iostream>
#include <cmath>

#include "../quad.hpp"
#include "../index.hpp"
#include "../random.hpp"
#include "../grid.hpp"
#include "../cell.hpp"
#include "../dynamic_sim.hpp"

#define COLUMNS 512
#define ROWS 512
#define CELL_SIZE 2

constexpr double pi() { return std::atan(1) * 4; }

enum Type { Ant, Cell };

namespace Langton {
class Agent {
    bool alive = false;
    int angle = 0; // degrees

public:
    using Cell = CASE::ZCell<Agent, 2>;

    int z = 0;

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
        z = 1;
    else
        z = 0;
}

void Agent::update() {
    if (z == 0) {
        auto neighbors = cell->neighbors();
        constexpr static int cell_layer = 1;
        constexpr static int turn_amt = 90;
        auto cell = neighbors(0, 0).getlayer(cell_layer);
        if (cell == nullptr) {
            neighbors(0, 0).spawn(Agent{Type::Cell});
            // turn right
            angle -= turn_amt;
        }
        else {
            cell->deactivate();
            // turn left
            angle += turn_amt;
        }
        // move forward
        const int x = std::round(std::cos(angle * pi() / 180)),
                  y = std::round(std::sin(angle * pi() / 180));
        neighbors(x, y).insert(this);
    }
}

void Agent::draw(const int x, const int y, std::vector<sf::Vertex> & vs) const {
    const auto i = vs.size();
    vs.resize(vs.size() + 4);

    int r = 0, g = 0, b = 0;
    if (z == 0)
        r = 200;

    CASE::quad(x * CELL_SIZE, y * CELL_SIZE,
            CELL_SIZE, CELL_SIZE, r,g,b, &vs[i]);
}
}

struct Config {
    using Agent = Langton::Agent;
    using Cell = Agent::Cell;
    using Grid = CASE::Grid<Cell>;

    static constexpr int columns = COLUMNS;
    static constexpr int rows = ROWS;
    static constexpr int cell_size = CELL_SIZE;
    static constexpr int subset = columns * rows * Cell::depth;

    const double framerate = 20;
    const char* title = "Langtons Ant";
    const sf::Color bgcolor{255, 255, 255};

    void init(Grid & grid, CASE::AgentManager<Agent> & manager) {
        grid.clear();
        manager.clear();

        //CASE::Uniform<0, columns - 1> x;
        //CASE::Uniform<0, rows - 1> y;

        //for (auto i = 0; i < subset/2; i++)
            //grid(x(), y()).spawn(Agent{None});

        grid(COLUMNS/2, ROWS/2).spawn(Agent{Ant});
        grid(COLUMNS/2+1, ROWS/2-1).spawn(Agent{Ant});
        grid(COLUMNS/2-1, ROWS/2+1).spawn(Agent{Ant});
        grid(COLUMNS/2-1, ROWS/2-1).spawn(Agent{Ant});
    }

    void postprocessing(Grid & /*grid*/) {
    }
};

int main() {
    CASE::Dynamic<Config>();
}

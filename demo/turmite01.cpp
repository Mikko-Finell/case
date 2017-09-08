#include <iostream>
#include <cmath>

#include "../quad.hpp"
#include "../index.hpp"
#include "../random.hpp"
#include "../grid.hpp"
#include "../cell.hpp"
#include "../dynamic_sim.hpp"

#define COLUMNS 256
#define ROWS 256
#define CELL_SIZE 3
#define CELL_LAYER 1
#define ANT_LAYER 0

constexpr double pi() { return std::atan(1) * 4; }

enum Type { Ant, Cell };
enum CellState { First, Second };

namespace Langton {
class Agent {
    bool alive = false;
    int angle = 0; // degrees
    CellState state = CellState::First;

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
    void setstate(const CellState s) { state = s; }
};

Agent::Agent(const Type _type) {
    if (_type == Type::Cell) {
        z = CELL_LAYER;
        state = CellState::First;
    }
    else {
        static CASE::Uniform<0,3> rand;
        angle = 90 * rand();
        z = ANT_LAYER;
    }
}

void Agent::update() {
    if (z == ANT_LAYER) {
        auto neighbors = cell->neighbors();
        constexpr static int turn_amt = -90;

        auto cell = neighbors(0, 0).getlayer(CELL_LAYER);
        if (cell == nullptr) {
            neighbors(0, 0).spawn(Agent{Type::Cell});
            angle += turn_amt;
        }
        else {
            if (cell->state == CellState::First) {
                angle += turn_amt;
                cell->state = CellState::Second;
            }
            else {
                angle -= turn_amt;
                cell->deactivate();
            }
        }
        // move forward
        neighbors(
            //std::cos(angle * pi() / 180),
            //std::sin(angle * pi() / 180)
            std::round(std::cos(angle * pi() / 180)),
            std::round(std::sin(angle * pi() / 180))
        ).insert(this);
    }
}

void Agent::draw(const int x, const int y, std::vector<sf::Vertex> & vs) const {
    const auto i = vs.size();
    vs.resize(vs.size() + 4);

    int r = 0, g = 0, b = 0;
    if (z == 0)
        r = 200;
    else if (state == CellState::Second) {
        r = 150; g = 150; b = 150;
    }

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

        CASE::Uniform<0, columns - 1> x;
        CASE::Uniform<0, rows - 1> y;

        //for (auto i = 0; i < 1; i++)
            //grid(x(), y()).spawn(Agent{Ant});

        /*
        for (auto y = 0; y < rows; y++) {
            for (auto x = 0; x < columns; x++) {
                Agent * cell = grid(x, y).spawn(Agent{Type::Cell});
                if (cell != nullptr)
                    cell->setstate(CellState::First);
            }
        }
        */

        grid(COLUMNS/2, ROWS/2).spawn(Agent{Ant});
    }

    void postprocessing(Grid & /*grid*/) {
    }
};

int main() {
    CASE::Dynamic<Config>();
}

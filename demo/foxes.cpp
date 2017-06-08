#include <iostream>

#include "../quad.hpp"
#include "../index.hpp"
#include "../random.hpp"
#include "../grid.hpp"
#include "../cell.hpp"
#include "../dynamic_sim.hpp"

#define COLUMNS 200
#define ROWS 200
#define CELL_SIZE 2

enum Type { Fox, Rabbit, Grass, None };

namespace FoxesAndRabbits {
class Agent {
    bool alive = false;

public:
    using Cell = CASE::ZCell<Agent, 2>;

    Type type;
    int z = 0;
    int max_energy = 255;
    int energy = max_energy;

    Cell * cell = nullptr;

    Agent(Type type = Type::None);
    void update();
    void draw(const int, const int, std::vector<sf::Vertex> & vertices) const;

    bool active() const { return alive; }
    void activate() { alive = true; }
    void deactivate() {
        alive = false;
    }
};

Agent::Agent(const Type _type) {
    static CASE::Uniform<0, 100> dist;
    if (_type == None) {
        auto r = dist();
        if (r > 99)
            type = Fox;
        else if (r > 90)
            type = Rabbit;
        else
            type = Grass;
    }
    else
        this->type = _type;
    if (type == Grass) {
        z = 1;
        energy = 50;
    }
    else if (type == Rabbit) {
        z = 0;
        energy = 255;
    }
    else {
        z = 0;
        energy = 200;
    }
}

void Agent::update() {
    if (energy <= 0)
        return deactivate();

    auto neighbors = cell->neighbors();

    static CASE::Uniform<-1, 1> uv;
    static CASE::Uniform<0, 100> rand_percent{};

    if (type == Grass) {
        auto grass = neighbors(uv(), uv()).getlayer(1);
        if (grass != nullptr) {
            grass->energy += 2;
            if (grass->energy > max_energy)
                grass->energy = max_energy;
        }
        else if (energy > 0.2 * max_energy)
            neighbors(uv(), uv()).spawn(Agent{Grass});
    }
    else if (type == Rabbit) {
        energy -= 6;

        bool breed = rand_percent() < 10 && energy > 0.7 * max_energy;
        if (breed) {
            auto rabbit = neighbors(uv(), uv()).spawn(Agent{Rabbit});
            if (rabbit != nullptr) {
                rabbit->energy = energy;
                energy -= 20;
            }
        }
        auto grass = cell->getlayer(1);
        if (grass != nullptr) {
            energy += grass->energy;
            grass->energy -= 50;
        }
        neighbors(uv(), uv()).insert(this);
    }
    else { // type is Fox
        energy -= 10;
        
        const auto breed = rand_percent() < 5 && energy > 0.75 * max_energy;
        if (breed) {
            if (neighbors(uv(), uv()).spawn(Agent{Fox}) != nullptr)
                energy -= 10;
        }
        for (Cell * cellptr : neighbors.cells()) { 
            auto ptr = cellptr->getlayer(0);
            if (ptr == nullptr)
                continue;

            auto & agent = *ptr;
            if (agent.type == Rabbit) {
                energy += 50;
                agent.energy = 0;
                break;
            }
        }
        neighbors(uv(), uv()).insert(this);
    }
}

void Agent::draw(const int x, const int y, std::vector<sf::Vertex> & vs) const {
    const auto i = vs.size();
    vs.resize(vs.size() + 4);

    int r = 0, g = 0, b = 0;
    if (type == Grass)
        g = -0.5 * energy + 230;

    else if (type == Rabbit) {
        g = 155;
        b = 255;
    }
    else {
        r = 255;
        g = 100;
    }

    CASE::quad(x * CELL_SIZE, y * CELL_SIZE,
            CELL_SIZE, CELL_SIZE, r,g,b, &vs[i]);
}
} // FoxesAndRabbits

struct Config {
    using Agent = FoxesAndRabbits::Agent;
    using Cell = Agent::Cell;
    using Grid = CASE::Grid<Cell>;

    static constexpr int columns = COLUMNS;
    static constexpr int rows = ROWS;
    static constexpr int cell_size = CELL_SIZE;
    static constexpr int subset = columns * rows * Cell::depth;

    const double framerate = 20;
    const char* title = "Foxes and Rabbits";
    const sf::Color bgcolor{0,0,0};

    void init(Grid & grid, CASE::AgentManager<Agent> & manager) {
        grid.clear();
        manager.clear();

        CASE::Uniform<0, columns - 1> x;
        CASE::Uniform<0, rows - 1> y;

        for (auto i = 0; i < subset/2; i++)
            grid(x(), y()).spawn(Agent{None});
    }

    void postprocessing(Grid & /*grid*/) {
    }
};

int main() {
    CASE::Dynamic<Config>();
}

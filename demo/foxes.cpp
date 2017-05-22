#include <iostream>

#include "../quad.hpp"
#include "../index.hpp"
#include "../random.hpp"
#include "../grid.hpp"
#include "../cell.hpp"
#include "../simulator.hpp"

#define COLUMNS 256
#define ROWS 256
#define CELL_SIZE 3

int rng(const int low, const int high) {
    static CASE::Random rng;
    return rng(low, high);
}

template<int low, int high>
auto range() {
    static CASE::Random rng;
    return rng.range<low, high>();
}

struct UnitVector {
    const int x; const int y;
};

UnitVector random_direction() {
    return { rng(-1, 1), rng(-1, 1) };
}

int foxes = 0;
int rabbits = 0;
int grasses = 0;

enum Type { Fox, Rabbit, Grass, None };

namespace FoxesAndRabbits {
class Agent {
    bool alive = false;

public:
    using Cell = CASE::ZCell<Agent, 2>;

    Type type;
    int x = 0, y = 0, z = 0;
    int max_energy = 255;
    int energy = max_energy;

    Cell * cell = nullptr;

    Agent(Type type = Type::None);
    void update(CASE::Grid<Cell> & grid);
    void draw(std::vector<sf::Vertex> & vertices) const;

    void setpos(const int _x, const int _y) {
        x = CASE::wrap(_x, COLUMNS);
        y = CASE::wrap(_y, ROWS);
    }
    void relative_move(const int _x, const int _y) {
        x = CASE::wrap(x + _x, COLUMNS);
        y = CASE::wrap(y + _y, ROWS);
    }
    bool active() const { return alive; }
    void activate() { alive = true; }
    void deactivate() { alive = false; }
};

Agent::Agent(const Type _type) {
    if (_type == None) {
        auto r = rng(0, 100);
        if (r > 90)
            type = Fox;
        else if (r > 70)
            type = Rabbit;
        else
            type = Grass;
    }
    else
        this->type = _type;

    if (type == Fox) {
        energy = 200;
        z = 0;
    }
    else if (type == Rabbit) {
        energy = 255;
        z = 0;
    }
    else {
        energy = 50;
        z = 1;
    }
}

void Agent::update(CASE::Grid<Cell> & grid) {
    if (!alive)
        return;

    if (energy <= 0) {
        deactivate();
        if (type == Grass)
            --grasses;
        else if (type == Rabbit)
            --rabbits;
        else
            --foxes;
        return;
    }

    const auto vec = random_direction();
    auto neighbors = cell->neighbors();

    if (type == Grass) {

        Cell & cell = neighbors(vec.x, vec.y);

        auto grass_neighbor = cell.getlayer(1);
        if (grass_neighbor != nullptr) {
            grass_neighbor->energy += 3;
            if (grass_neighbor->energy > max_energy)
                grass_neighbor->energy = max_energy;
        }

        if (energy > 0.3 * max_energy) {
            Agent grass{Grass};
            grass.z = 1;
            grass.setpos(x + vec.x, y + vec.y);
            if (grid.spawn(grass) == CASE::OK)
                ++grasses;
        }
    }

    else if (type == Rabbit) {

        energy -= 1;
        bool breed = rng(1, 20) < 2 && energy > 0.5 * max_energy;

        auto & cell = neighbors(vec.x, vec.y);

        if (breed) {
            Agent kit{Rabbit};
            kit.z = 0;
            kit.setpos(x + vec.x, y + vec.y);
            kit.energy = energy;

            auto code = grid.spawn(kit);
            if (code == CASE::OK) {
                energy -= 20;
                ++rabbits;
            }
        }

        auto grass = cell.getlayer(1);
        if (grass!= nullptr) {
            energy += grass->energy;
            grass->energy -= 50;
        }

        const auto dir = random_direction();
        grid.move(this, dir.x, dir.y);
    }

    else { // type is Fox

        energy -= 10;

        const auto breed = rng(1, 100) < 5 && energy > 0.7 * max_energy;

        auto neighbors = cell->neighbors();
        if (breed) {
            const auto vec = random_direction();
            auto & cell = neighbors(vec.x, vec.y);

            Agent cub{Fox};
            cub.z = 0;
            cub.setpos(x + vec.x, y + vec.y);

            if (grid.spawn(cub) == CASE::OK) {
                energy -= 20;
                ++foxes;
            }
        }

        auto cellpointers = neighbors.cells();
        for (Cell * cellptr : cellpointers) { 
            auto ptr = cellptr->getlayer(0);
            if (ptr == nullptr)
                continue;

            auto & agent = *ptr;
            if (agent.type == Rabbit) {
                energy += 50;
                agent.energy = 0;
            }
        }

        const auto dir = random_direction();
        grid.move(this, dir.x, dir.y);
    }
}

void Agent::draw(std::vector<sf::Vertex> & vs) const {
    const auto i = vs.size();
    vs.resize(vs.size() + 4);

    int r = 0, g = 0, b = 0;
    if (type == Grass)
        g = -0.5 * energy + 250;

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
    static constexpr int max_agents = columns * rows * 2;
    static constexpr int subset = max_agents * 1;
    const double framerate = 60;
    const char* title = "Foxes and Rabbits";
    const sf::Color bgcolor{0,0,0}; //= sf::Color{147, 113, 66};

    void init(Grid & grid) {
        grid.clear();

        for (auto i = 0 ; i < max_agents * 0.1 ; i++) {
            const auto x = rng(0, columns - 1);
            const auto y = rng(0, rows - 1);

            Agent agent{None};
            agent.setpos(x, y);
            grid(x, y).insert(agent);

            switch (agent.type) {
                case Grass: grasses++; break;
                case Rabbit: rabbits++; break;
                default: foxes++; break;
            }
        }
    }

    void postprocessing(Grid & grid) {
        static CASE::Log log{"data/pop.dat"};
        log.out(foxes+rabbits+grasses);
    }
};

int main() {
    CASE::simulator::Dynamic<Config>();
}

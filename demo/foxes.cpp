#include <iostream>

#include "../quad.hpp"
#include "../index.hpp"
#include "../random.hpp"
#include "../grid.hpp"
#include "../cell.hpp"
#include "../simulator.hpp"

// these were used to collect 'before' data
#define COLUMNS 256
#define ROWS 256
#define CELL_SIZE 2

//#define COLUMNS 100
//#define ROWS 100
//#define CELL_SIZE 4

int rng(const int low, const int high) {
    static CASE::Random random;
    return random(low, high);
}

template<int low, int high>
auto range() {
    static CASE::Random random;
    return random.range<low, high>();
}

struct UnitVector {
    const int x; const int y;
};

UnitVector random_direction() {
    static CASE::RDist<-1, 1> dist;
    return { dist(), dist() };
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
    static CASE::RDist<1, 100> dist;
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

    if (type == Fox) {
        foxes++;
        energy = 200;
        z = 0;
    }
    else if (type == Rabbit) {
        rabbits++;
        energy = 255;
        z = 0;
    }
    else {
        grasses++;
        energy = 50;
        z = 1;
    }
}

void Agent::update() {
    if (!alive) {
        return;
    }

    if (energy <= 0) {
        deactivate();
        //cell->extract(this->z);
        return;
    }

    const auto vec = random_direction();
    auto neighbors = cell->neighbors();

    if (type == Grass) {
        assert(z == 1);

        auto grass_neighbor = neighbors(vec.x, vec.y).getlayer(1);
        if (grass_neighbor != nullptr) {
            grass_neighbor->energy += 2;
            if (grass_neighbor->energy > max_energy)
                grass_neighbor->energy = max_energy;
        }

        else if (energy > 0.3 * max_energy) {

            auto grass = neighbors(vec.x, vec.y).spawn(Agent{Grass});
            if (grass != nullptr) {
                assert(grass->type == Grass);
                assert(grass->z == 1);
                assert(grass->cell == &neighbors(vec.x, vec.y));
                assert(neighbors(vec.x, vec.y).getlayer(z) == grass);
            }
        }
    }
    else if (type == Rabbit) {
        assert(z == 0);
        static CASE::RDist<1, 20> dist_1_to_20;
        energy -= 6;
        bool breed = dist_1_to_20() < 2 && energy > 0.7 * max_energy;

        if (breed) {
            auto rabbit = neighbors(vec.x, vec.y).spawn(Agent{Rabbit});
            if (rabbit != nullptr) {
                assert(rabbit->type == Rabbit);
                assert(rabbit->z == 0);
                assert(rabbit->cell == &neighbors(vec.x, vec.y));
                assert(neighbors(vec.x, vec.y).getlayer(z) == rabbit);

                rabbit->energy = energy;
                energy -= 20;
            }
        }
        auto grass = cell->getlayer(1);
        if (grass != nullptr) {
            energy += grass->energy;
            grass->energy -= 50;
        }
        const auto dir = random_direction();
        neighbors(dir.x, dir.y).insert(this);
    }

    else { // type is Fox
        assert(z == 0);
        static CASE::RDist<1, 100> dist_1_to_100{};
        energy -= 10;
        const auto breed = dist_1_to_100() < 5 && energy > 0.75 * max_energy;

        if (breed) {
            auto fox = neighbors(vec.x, vec.y).spawn(Agent{Fox});
            if (fox != nullptr) {
                assert(fox->type == Fox);
                assert(fox->z == 0);
                assert(fox->cell == &neighbors(vec.x, vec.y));
                assert(neighbors(vec.x, vec.y).getlayer(z) == fox);

                energy -= 10;
            }
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
        const auto dir = random_direction();
        neighbors(dir.x, dir.y).insert(this);
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

    const double framerate = 60;
    const char* title = "Foxes and Rabbits";
    const sf::Color bgcolor{0,0,0};

    void init(Grid & grid, CASE::AgentManager<Agent> & manager) {
        grid.clear();
        manager.clear();

        for (auto i = 0; i < subset/2; i++){
            const auto x = rng(0, columns - 1);
            const auto y = rng(0, rows - 1);

            grid(x, y).spawn(Agent{None});
        }
    }

    void postprocessing(Grid & /*grid*/) {
    }
};

int main() {
    CASE::simulator::Dynamic<Config>();
}

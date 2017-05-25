#include <iostream>

#include "../quad.hpp"
#include "../index.hpp"
#include "../random.hpp"
#include "../grid.hpp"
#include "../cell.hpp"
#include "../simulator.hpp"

#define COLUMNS 150
#define ROWS 150
#define CELL_SIZE 3

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
    int z = 0;
    int max_energy = 255;
    int energy = max_energy;

    Cell * cell = nullptr;

    Agent(Type type = Type::None);
    void update(CASE::Grid<Cell> & grid);
    void draw(const int, const int, std::vector<sf::Vertex> & vertices) const;

    bool active() const { return alive; }
    void activate() { alive = true; }
    void deactivate() {
        alive = false;
    }
};

Agent::Agent(const Type _type) {
    if (_type == None) {
        auto r = rng(0, 100);
        if (r > 99)
            type = Fox;
        else if (r > 70)
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

void Agent::update(CASE::Grid<Cell> & grid) {
    if (!alive)
        return;

    if (energy <= 0) {
        grid.kill(*this);
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
        assert(z == 1);

        Cell & cell = neighbors(vec.x, vec.y);

        auto grass_neighbor = cell.getlayer(1);
        if (grass_neighbor != nullptr) {
            grass_neighbor->energy += 3;
            if (grass_neighbor->energy > max_energy)
                grass_neighbor->energy = max_energy;
        }

        else if (energy > 0.3 * max_energy) {
            Agent grass{Grass};
            grass.z = 1;
            auto ptr = grid.spawn(grass, cell);
            if (ptr != nullptr) {
                assert(ptr->type == Grass);
                assert(ptr->z == 1);
            }
        }
    }

    else if (type == Rabbit) {
        assert(z == 0);

        energy -= 4;
        bool breed = rng(1, 20) < 2 && energy > 0.6 * max_energy;

        auto & cell = neighbors(vec.x, vec.y);

        if (breed) {
            Agent kit{Rabbit};
            kit.z = 0;
            kit.energy = energy;
            auto ptr = grid.spawn(kit, cell);
            if (ptr != nullptr) {
                assert(ptr->type == Rabbit);
                ptr->energy = energy;
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
        neighbors(dir.x, dir.y).insert(this);
    }

    else { // type is Fox
        assert(z == 0);

        energy -= 10;
        const auto breed = rng(1, 100) < 5 && energy > 0.75 * max_energy;

        if (breed) {
            Agent cub{Fox};
            cub.z = 0;
            auto ptr = grid.spawn(cub, cell->x+vec.x, cell->y+vec.y);

            if (ptr != nullptr) {
                assert(ptr->type == Fox);
                //energy -= 20;
                ++foxes;
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
    static constexpr int subset = columns * rows * Cell::depth;

    const double framerate = 60;
    const char* title = "Foxes and Rabbits";
    const sf::Color bgcolor{0,0,0};

    void init(Grid & grid) {
        grid.clear();

        for (auto i = 0; i < subset; i++){
            const auto x = rng(0, columns - 1);
            const auto y = rng(0, rows - 1);

            Agent agent{None};
            auto ptr = grid.spawn(agent, x, y);

            if (ptr == nullptr)
                continue;

            switch (agent.type) {
                case Grass: grasses++; break;
                case Rabbit: rabbits++; break;
                default: foxes++; break;
            }
        }
    }

    void postprocessing(Grid & grid) {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::P)) {
            std::cout << "   ";
            for (auto x = 0; x < COLUMNS; x++) {
                if (x < 10) std::cout << " " ;
                std::cout << " " << x;
            }
            std::cout << '\n' ;

            for (auto y = 0; y < ROWS; y++) {
                if (y < 10) std::cout << " ";
                std::cout << " " << y ;
                for (auto x = 0; x < COLUMNS; x++) {
                    if (grid(x, y).getlayer(0) == nullptr)
                        
                        std::cout<<"  .";

                    else
                    {
                        auto & agent = *grid(x, y).getlayer(0);
                        if (agent.type == Rabbit)
                            std::cout << "  R";
                        if (agent.type == Fox)
                            std::cout << "  F";
                    }
                }
                std::cout<<'\n';
            }
            std::cout << "\n";
            /*
            for (auto i = 0; i < COLUMNS*ROWS*2; i++) {
                auto & agent = grid.agents[i];
                if (agent.active() == false) continue;

                assert(agent.cell != nullptr);

                if (agent.type == Grass)
                    std::cout << "Grass";
                if (agent.type == Rabbit)
                    std::cout << "Rabbit";
                if (agent.type == Fox)
                    std::cout << "Fox";

                std::cout << " position " << 
                    agent.cell->x << ", " << agent.cell->y << std::endl;
            }
            */
        }
        //static CASE::Log log{"data/pop.dat"};
        //log.out(foxes+rabbits+grasses);
    }
};

int main() {
    CASE::simulator::Dynamic<Config>();
}

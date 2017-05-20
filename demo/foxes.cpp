#include "../quad.hpp"
#include "../index.hpp"
#include "../random.hpp"
#include "../cell.hpp"
#include "../simulator.hpp"
#include "../world.hpp"

#define COLUMNS 200
#define ROWS 200
#define CELL_SIZE 5

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
    /*
    static const int dir[4][2] = {
        {1, 0},{-1, 0}, {0, -1},{0, 1}
    };
    const auto r = rng(0, 3);
    return UnitVector{ dir[r][0], dir[r][1] };
    */
    return { rng(-1, 1), rng(-1, 1) };
}

int foxes = 0;
int rabbits = 0;
int grasses = 0;

enum Type { Fox, Rabbit, Grass, None };

namespace FoxesAndRabbits {
class Agent {
private:
    bool alive = false;

public:
    Type type;
    int x = 0, y = 0, z = 0;
    int max_energy = 255;
    int energy = max_energy;

    static Agent * world;
    CASE::ZCell<Agent, 2> * cell = nullptr;

    using World = CASE::World<Agent, CASE::Grid<CASE::ZCell<Agent, 2>>>;

    Agent(Type type = Type::None);

    void update(World & world);
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
        else 
            type = Rabbit;
    }
    else
        this->type = _type;

    if (type == Fox)
        energy = 200;
    else if (type == Rabbit)
        energy = 255;
    else
        energy = 100;
}

void Agent::update(Agent::World & world) {
    if (!alive)
        return;

    if (energy <= 0) {
        const auto code = world.kill(*this);
        assert(code == CASE::Code::OK);
        if (type == Grass)
            --grasses;
        else if (type == Rabbit)
            --rabbits;
        else
            --foxes;
        return;
    }

    if (type == Grass) {
        auto & grass = *this;

        const auto idle = rng(1, 10) > 5;
        if (idle)
            return; 

        auto & neighbors = grass.cell->neighbors;
        const auto vec = random_direction();
        auto & cell = neighbors.cell(vec.x, vec.y);

        auto grass_neighbor = cell.getlayer(1);
        if (grass_neighbor != nullptr) {
            grass_neighbor->energy += 10;
            if (grass_neighbor->energy > grass.max_energy)
                grass_neighbor->energy = grass.max_energy;
        }
        else if (grass.energy > 0.5 * grass.max_energy) {
            Agent grass_model{Grass};
            grass_model.z = 1;
            grass_model.setpos(grass.x + vec.x, grass.y + vec.y);
            auto new_grass = world.try_spawn(grass_model);
            if (new_grass != nullptr) {
                ++grasses;
            }
        }
    }

    else if (type == Rabbit) {
        auto & rabbit = *this;
        //rabbit.energy -= 5;

        bool breed = rng(1, 20) < 2
                     && rabbit.energy > 0.5 * rabbit.max_energy;

        if (breed) {
            auto & neighbors = rabbit.cell->neighbors;
            const auto vec = random_direction();
            auto & cell = neighbors.cell(vec.x, vec.y);

            Agent rabbit_model{Rabbit};
            rabbit_model.z = 0;
            rabbit_model.setpos(rabbit.x + vec.x, rabbit.y + vec.y);
            auto kit = world.try_spawn(rabbit_model);
            if (kit != nullptr) {
                //rabbit.energy *= 0.5;
                ++rabbits;
            }
        }

        const auto vec = random_direction();
        world.try_move(rabbit, rabbit.x + vec.x, rabbit.y + vec.y);
    }

    else {
        auto & fox = *this;
        fox.energy -= 10;

        const auto breed = rng(1, 100) < 5
                           && fox.energy > 0.7 * fox.max_energy;

        auto & neighbors = fox.cell->neighbors;
        if (breed) {

            const auto vec = random_direction();
            auto & cell = neighbors.cell(vec.x, vec.y);

            Agent cubmodel{Fox};
            cubmodel.z = 0;
            cubmodel.setpos(fox.x + vec.x, fox.y + vec.y);

            if (world.try_spawn(cubmodel) != nullptr) {
                fox.energy -= 50;
                ++foxes;
            }
        }

        Agent * prey = nullptr;

        static const int range[3] = {-1, 0, 1};
        for (const auto Y : range) {
            for (const auto X : range) {
                auto neighbor = neighbors(X, Y);
                if (neighbor != nullptr && neighbor->type == Rabbit) {
                    prey = neighbor;
                    break;
                }
            }
        }

        if (prey == nullptr) {
            const auto vec = random_direction();
            world.try_move(fox, fox.x + vec.x, fox.y + vec.y);
        }
        else {
            fox.energy += 50;
            prey->energy = -200;
        }
    }
}

void Agent::draw(std::vector<sf::Vertex> & vs) const {
    const auto i = vs.size();
    vs.resize(vs.size() + 4);
    if (type == Grass) {
        vs[i].color.a = 100;
        vs[i+1].color.a = 100;
        vs[i+2].color.a = 100;
        vs[i+3].color.a = 100;
    }
    const auto r = type == Fox ? 200 : 0;
    const auto g = type == Rabbit ? 200 : type == Grass
        ? 100 + (energy > 255 ? 255 : energy < 0 ? 0 : energy) : 0;
    const auto b = type == Rabbit ? 200 : 0;
    CASE::quad(x * CELL_SIZE, y * CELL_SIZE,
            CELL_SIZE, CELL_SIZE, r,g,b, &vs[i]);
}
} // FoxesAndRabbits

struct Config {
    using Agent = FoxesAndRabbits::Agent;
    using Cell = CASE::ZCell<FoxesAndRabbits::Agent, 2>;

    static constexpr int columns = COLUMNS;
    static constexpr int rows = ROWS;
    static constexpr int cell_size = CELL_SIZE;
    static constexpr int max_agents = columns * rows * 2;
    static constexpr int subset = max_agents * 1;
    const double framerate = 60;
    const char* title = "Foxes and Rabbits";
    const sf::Color bgcolor = sf::Color{147, 113, 66};

    void init(Agent::World & world) {
        world.clear();

        for (auto i = 0 ; i < max_agents * 0.1 ; i++) {
        //for (auto i = 0 ; i < 1 ; i++) {
            const auto x = rng(0, columns - 1);
            const auto y = rng(0, rows - 1);

            if (world.grid(x, y).is_occupied())
                continue;

            Agent agent{None};
            agent.setpos(x, y);
            if (world.try_spawn(agent) != nullptr) {
                switch (agent.type) {
                    case Grass: grasses++; break;
                    case Rabbit: rabbits++; break;
                    default: foxes++; break;
                }

            }
        }
        world.sort();
    }

    void postprocessing(Agent::World & world) {
        static CASE::Log flog{"foxes.dat"};
        static CASE::Log rlog{"rabbits.dat"};
        static CASE::Log glog{"grasses.dat"};
        flog.out(foxes);
        rlog.out(rabbits);
        glog.out(grasses);
    }
};

int main() {
    CASE::simulator::Dynamic<Config>();
}

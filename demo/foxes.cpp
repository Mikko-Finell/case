#include "../quad.hpp"
#include "../index.hpp"
#include "../random.hpp"
#include "../cell.hpp"
#include "../simulator.hpp"
#include "../world.hpp"

#define COLUMNS 192
#define ROWS 108
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

int foxes = 0;
int rabbits = 0;
int grasses = 0;

enum Type { Fox = 0, Rabbit = 1, Grass = 2 };

class Agent {
private:
    bool alive = false;

public:
    Type type;
    int x = 0, y = 0; //, z = 0;
    int max_energy = 255;
    int energy = max_energy;

    static Agent * world;
    CASE::ZCell<Agent, 2> * cell = nullptr;

    using World = CASE::World<CASE::ZCell<Agent, 2>>;

    Agent();

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

Agent * get_nearby(Agent & agent, const Type t) {
    assert(agent.active());
    assert(agent.cell != nullptr);

    auto & neighbors = agent.cell->neighbors;
    static const int direction[4][2] = {
        {0, 1}, {0, -1}, {-1, 0}, {1, 0}
    };
    for (const auto i : range<0, 3>()) {
        const auto dir = direction[i];
        const auto x = dir[0], y = dir[1];
        auto neighbor = neighbors(x, y);
        if (neighbor != nullptr && neighbor->type == t)
            return neighbor;
    }
    return nullptr;
}

Agent * spawn(Type type, Agent::World & world) {
    auto ptr = world.spawn();
    if (ptr == nullptr) return ptr; // max agents reached
    
    auto & agent = *ptr;
    agent = Agent{};
    agent.type = type;

    static CASE::Log grasslog{"grass.dat"};
    static CASE::Log rabbitlog{"rabbits.dat"};
    static CASE::Log foxlog{"foxes.dat"};
    grasslog.out(grasses);
    rabbitlog.out(rabbits);
    foxlog.out(foxes);

    return ptr;
}

void heal(Agent & agent, const int amount) {
    agent.energy += amount;
    if (agent.energy > agent.max_energy)
        agent.energy = agent.max_energy;
}

void move(Agent & agent) {
    int x = rng(-1, 1);
    int y = rng(-1, 1);

    assert(agent.active());
    assert(agent.cell != nullptr);
    assert(agent.cell->get(0) == &agent);

    auto & neighbors = agent.cell->neighbors;
    if (neighbors.cell(x, y).get(0) != nullptr)
        return;

    auto & target_cell = neighbors.cell(x, y);
    agent.cell->extract(0);
    const auto code = target_cell.insert(agent, 0);
    assert(code == CASE::Code::OK);

    agent.relative_move(x, y);
}

void breed(Agent & agent) {
    assert(agent.cell != nullptr);
    auto nearby_kin = get_nearby(agent, agent.type);
    if (nearby_kin == nullptr)
        return;

    // else add new agent to map
}

void update_rabbit(Agent & rabbit, Agent::World & world) {
    enum Action { Move, Graze, Breed };

    rabbit.energy -= 5;
    if (rabbit.energy <= 0) {
        rabbits--;
        assert(world.kill(rabbit) == CASE::Code::OK);
        return;
    }

    auto & neighbors = rabbit.cell->neighbors;
    if (rng(1, 20) == 1)
        goto BREED;
    else
        goto GRAZE;
    {
BREED:
    if (rabbit.energy < 0.5 * rabbit.max_energy)
        return move(rabbit);

    auto & neighbors = rabbit.cell->neighbors;
    static const int dir[4][2] = {
        {1, 0},{-1, 0}, {0, -1},{0, 1}
    };

    const auto r = rng(0, 3);
    const auto x = dir[r][0], y = dir[r][1];

    auto & cell = neighbors.cell(x, y);

    auto kit = spawn(Rabbit, world);
    if (kit == nullptr)
        return;

    kit->setpos(
        CASE::wrap(rabbit.x + x, COLUMNS),
        CASE::wrap(rabbit.y + y, ROWS)
    );
    const auto code = cell.insert(kit, 0);
    if (code == CASE::Code::OK) {
        kit->activate();
        rabbit.energy *= 0.5;
        rabbits++;
    }
    return;
    }

    {
GRAZE:
    assert(rabbit.cell != nullptr);
    Agent * grass = rabbit.cell->get(1);
    if (grass == nullptr)
        return move(rabbit);

    rabbit.energy += 50;
    grass->energy -= 50;
    }
}

void update_fox(Agent & fox, Agent::World & world) {
    enum Action { Move, Hunt, Breed };

    fox.energy -= 3;
    if (fox.energy <= 0) {
        assert(world.kill(fox) == CASE::Code::OK);
        foxes--;
        return;
    }

    if (rng(1, 20) == 3)
        goto BREED;
    else
        goto HUNT;

    {
BREED:
    if (fox.energy < 0.5 * fox.max_energy)
        goto HUNT;

    auto & neighbors = fox.cell->neighbors;
    static const int dir[4][2] = {
        {1, 0},{-1, 0}, {0, -1},{0, 1}
    };

    const auto r = rng(0, 3);
    const auto x = dir[r][0], y = dir[r][1];

    auto & cell = neighbors.cell(x, y);

    auto cub = spawn(Fox, world);
    if (cub == nullptr)
        return;

    cub->setpos(
        CASE::wrap(fox.x + x, COLUMNS),
        CASE::wrap(fox.y + y, ROWS)
    );
    const auto code = cell.insert(cub, 0);
    if (code == CASE::Code::OK) {
        cub->activate();
        fox.energy *= 0.5;
        foxes++;
    }
    return;
       }

HUNT:
    assert(fox.cell != nullptr);
    auto prey = get_nearby(fox, Rabbit);
    if (prey == nullptr)
        return move(fox);

    fox.energy += 50;
    prey->energy = -200;

    return;
}

void update_grass(Agent & grass, Agent::World & world) {
    auto & neighbors = grass.cell->neighbors;
    assert(grass.cell != nullptr);

    if (grass.energy <= 0) {
        assert(world.kill(grass) == CASE::Code::OK);
        grasses--;
        return;
    }

    static const int dir[4][2] = {
        {1, 0},{-1, 0}, {0, -1},{0, 1}
    };

    const auto r = rng(0, 3);
    const auto x = dir[r][0], y = dir[r][1];

    auto & cell = neighbors.cell(x, y);

    if (rng(0, 20) > 4)
        return;

    auto grass_neighbor = cell.get(1);
    if (grass_neighbor != nullptr)
        heal(*grass_neighbor, 10);

    else {
        if (grass.energy < 0.5 * grass.max_energy)
            return;

        auto new_grass = spawn(Grass, world);
        if (new_grass == nullptr)
            return;

        new_grass->setpos(
            CASE::wrap(grass.x + x, COLUMNS),
            CASE::wrap(grass.y + y, ROWS)
        );
        const auto code = cell.insert(new_grass, 1);
        if (code == CASE::Code::OK) {
            new_grass->activate();
            grasses++;
        }
    }
}

Agent::Agent() {
    auto r = rng(0, 100);
    if (r > 90) {
        type = Fox;
        energy = 200;
    }
    else if (r > 60) {
        type = Rabbit;
        energy = 255;
    }
    else {
        type = Grass;
        energy = 100;
    }
}

void Agent::update(Agent::World & world) {
    if (!active())
        return;

    switch (type) {
        case Rabbit: return update_rabbit(*this, world);
        case Fox:    return update_fox(*this, world);
        case Grass:  return update_grass(*this, world);
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

Agent * Agent::world = nullptr;

struct FoxesAndRabbits {
    using Agent = Agent;
    using Cell = CASE::ZCell<Agent, 2>;

    static constexpr int columns = COLUMNS;
    static constexpr int rows = ROWS;
    static constexpr int cell_size = CELL_SIZE;
    static constexpr int max_agents = columns * rows * 2;
    static constexpr int subset = max_agents;
    const double framerate = 30;
    const char* title = "Foxes and Rabbits";
    const sf::Color bgcolor = sf::Color{97+50, 63+50, 16+50};

    //std::function<bool(Agent &, Agent &)> predicate = [](Agent & a, Agent & b)
    //{ return a.z > b.z; };

    void init(Agent::World & world) {
        world.clear();

        const auto indices = range<0, max_agents>();
        for (auto i = 0 ; i < max_agents * 0.1 ; i++) {
        //for (auto i = 0 ; i < 1 ; i++) {
            const auto x = rng(0, columns - 1);
            const auto y = rng(0, rows - 1);

            auto & cell = world.grid(x, y);
            if (cell.is_occupied())
                continue;

            auto ptr = world.spawn();
            if (ptr == nullptr)
                continue;
            auto & agent = *ptr;

            agent.setpos(x, y);
            CASE::Code code;
            if (agent.type == Grass)
                code = cell.insert(agent, 1); 
            else
                code = cell.insert(agent, 0);

            if (code == CASE::Code::OK) {
                agent.activate();
                if (agent.type == Fox)
                    foxes++;
                else if (agent.type == Rabbit)
                    rabbits++;
                else
                    grasses++;
            }
        }
    }
};

int main() {
    CASE::simulator::Dynamic<FoxesAndRabbits>();
}

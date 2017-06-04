#include <iostream>

#include "../quad.hpp"
#include "../index.hpp"
#include "../random.hpp"
#include "../grid.hpp"
#include "../cell.hpp"
#include "../static_sim.hpp"

#define COLUMNS 100
#define ROWS 75
#define CELL_SIZE 12
#define NUM_GENES 3
#define GENE_SIZE (CELL_SIZE/NUM_GENES)

bool show_vegetation = true;
bool show_herbivores = true;
bool show_carnivores = true;

template <int MIN, int MAX>
int clamp(const int value) {
    return value < MIN ? MIN : value > MAX ? MAX : value;
}

int diff(const int a, const int b) {
    return std::abs(a - b) / 255.0;
}

int colordiff(int a[3], int b[3]) {
    const auto rd = diff(a[0],b[0]);
    const auto gd = diff(a[1],b[1]);
    const auto bd = diff(a[2],b[2]);
    return (rd + gd + bd) / 300.0;
}

bool decide(int gene) {
    static CASE::Uniform<1, 99> dist;
    return dist() <= 100.0 * (gene / 255.0);
}

int mutate(int gene, int factor) {
    static CASE::Cauchy rand;
    return clamp<0, 255>(rand(gene, factor));
}

enum Type { Vegetation, Herbivore, Carnivore };

#define VEGE_LAYER 1
#define HERB_LAYER 0
#define CARN_LAYER 0

class Organism {
    bool alive = false;
    Type type;

public:
    using Cell = CASE::ZCell<Organism, 2>;
    int z = 0;
    Cell * cell = nullptr;
    int gene[3] = {0, 0, 0};
    int hp = 0;

    void update();
    Organism() {}
    Organism(Type t);
    Organism(const Organism * parent);
    void draw(const int x, const int y, std::vector<sf::Vertex> & vs) {
        if (type == Vegetation && show_vegetation) {
            auto size = vs.size();
            vs.resize(size + 4);
            CASE::quad(cell->x*CELL_SIZE, cell->y*CELL_SIZE,
                    CELL_SIZE, CELL_SIZE, 0,255,0, &vs[size]);

            size += 4;
            vs.resize(size + 4);
            CASE::quad(cell->x*CELL_SIZE+1, cell->y*CELL_SIZE+1,
                    CELL_SIZE-2, CELL_SIZE-2, gene[0],gene[1],gene[2], &vs[size]);
        }

        else if (type == Herbivore && show_herbivores) {
            static const int pad = 2;
            auto size = vs.size();
            vs.resize(size + 4);
            CASE::quad(pad + x * CELL_SIZE, pad + y * CELL_SIZE,
                    CELL_SIZE - 2 * pad, CELL_SIZE - 2 * pad, 
                    gene[0],gene[1],gene[2], &vs[size]);
        }

        else if (type == Carnivore && show_carnivores) {
            static const int pad = 2;
            auto size = vs.size();
            vs.resize(size + 4);
            CASE::quad(pad + x * CELL_SIZE, pad + y * CELL_SIZE,
                    CELL_SIZE - 2 * pad, CELL_SIZE - 2 * pad, 
                    gene[0],gene[1],gene[2], &vs[size]);
        }
    }
    bool active() const { return alive; }
    void activate() { alive = true; }
    void deactivate() { alive = false; }
};

static CASE::Uniform<-1, 1> uv;
static CASE::Uniform<0,255> rcolor;
static CASE::Uniform<0,100> percent;

Organism::Organism(Type t) : Organism{this} {
    type = t;
    if      (type == Vegetation) { z = VEGE_LAYER; }
    else if (type == Herbivore)  { z = HERB_LAYER; }
    else if (type == Carnivore)  { z = CARN_LAYER; }
    for (auto i = 0; i < 3; i++) gene[i] = rcolor();
}

static const int vege_max_hp = 5000;
static const int herb_max_hp = 5000;
static const int carn_max_hp = 5000;

Organism::Organism(const Organism * parent) : type(parent->type), z(parent->z)
{
    if      (type == Vegetation) { hp = vege_max_hp; }
    else if (type == Herbivore)  { hp = herb_max_hp; }
    else if (type == Carnivore)  { hp = carn_max_hp; }
    static constexpr int mutation_factor = 10;
    for (auto i = 0; i < NUM_GENES; i++)
        gene[i] = mutate(parent->gene[i], mutation_factor);
}

void Organism::update() {
    for (auto i = 0; i < NUM_GENES; i++) {
        if (gene[i] > 100)
            hp -= gene[i];
    }
    if (hp <= 0) {
        deactivate();
        return;
    }
    auto neighbors = cell->neighbors();

    static const int cardinal[4][2] = {{0,-1},{0,1}, {-1,0},{1,0}};

    static const int vege_regen_gene = 0;
    static const int vege_growth_gene = 1;
    static const int vege_poison_gene = 2;
    static const int vege_z = 1, herb_z = 0, carn_z = 0;

    if (type == Vegetation) {
        if (decide(gene[vege_growth_gene]))
            neighbors(uv(), uv()).spawn(Organism{this});
        if (decide(gene[vege_regen_gene]))
            hp = clamp<0, vege_max_hp>(hp + vege_max_hp * 0.01);
    }

    static CASE::Uniform<0, 3> r;
    const auto i = r();
    const auto vx = cardinal[i][0], vy = cardinal[i][1];

    if (type == Herbivore) {
        static const int herb_chomp_dmg = vege_max_hp * 0.1;
        static const int herb_reprod_cost = herb_max_hp * 0.2;
        static const int herb_resist_gene = 2;
        static const int herb_reprod_gene = 1;
        static const int herb_move_gene = 0;
        static const int herb_move_cost = herb_max_hp * 0.015;

        auto plant = cell->getlayer(vege_z);
        if (plant != nullptr) {
            auto energy = plant->hp;
            if (decide(plant->gene[vege_poison_gene])) {
                if (decide(gene[herb_resist_gene]))
                    hp += energy * 0.5;
                else
                    hp *= 0.9;
            }
            else
                hp += energy * herb_move_gene / 255.0;
            plant->hp -= herb_chomp_dmg;
        }
        if (decide(gene[herb_move_gene])) {
            hp -= herb_move_cost;
            neighbors(vx, vy).insert(this);
            return;
        }
        if (decide(gene[herb_reprod_gene])) {
            hp -= herb_reprod_cost;
            neighbors(vx, vy).spawn(Organism{this});
        }
    }

    if (type == Carnivore) {
        static const int carn_reprod_cost = carn_max_hp * 0.8;
        static const int carn_move_cost = carn_max_hp * 0.015;
        static const int carn_hunt_gene = 0;
        static const int carn_resist_gene = 2;
        static const int carn_reprod_gene = 1;

        // red gene affect how many directions to hunt in per frame
        const int dirs = gene[carn_hunt_gene] / 64;
        for (auto i = 0; i < dirs; i++) {
            const auto x = cardinal[i][0], y = cardinal[i][1];
            auto prey = neighbors(x, y)[herb_z];
            if (prey != nullptr && prey->type == Herbivore) {
                hp = clamp<0, carn_max_hp>(hp + prey->hp);
                prey->hp = 0;
                return;
            }
        }

        auto plant = cell->getlayer(vege_z);
        if (plant != nullptr) {
            const auto vege_passive_dmg = plant->gene[vege_poison_gene] * 0.01;
            if (decide(gene[carn_resist_gene]) == false)
                hp -= vege_passive_dmg;
        }
        if (decide(gene[carn_hunt_gene])) {
            hp -= carn_move_cost;
            neighbors(vx, vy).insert(this);
            return;
        }
        if (hp > 0.85 * carn_max_hp && decide(gene[carn_reprod_gene])) {
            hp -= carn_reprod_cost;
            neighbors(vx, vy).spawn(Organism{this});
        }
    }
}

struct Config {
    using Agent = Organism;
    using Cell = Agent::Cell;
    using Grid = CASE::Grid<Cell>;

    static constexpr int columns = COLUMNS;
    static constexpr int rows = ROWS;
    static constexpr int cell_size = CELL_SIZE;
    static constexpr int subset = columns * rows * Cell::depth;

    const double framerate = 10;
    const char* title = "Color Evolution";
    const sf::Color bgcolor{0,0,0};

    void init(Grid & grid, CASE::AgentManager<Agent> & manager) {
        grid.clear();
        manager.clear();

        CASE::Uniform<0, columns - 1> xrange;
        CASE::Uniform<0, rows - 1> yrange;
        CASE::Uniform<1,3> r;
        for (auto i = 0; i < 0.01 * columns * rows; i++) {
            const auto type = r();
            if (type == 1)
                grid(xrange(), yrange()).spawn(Agent{Vegetation});
            else if (type == 2)
                grid(xrange(), yrange()).spawn(Agent{Herbivore});
            else
                grid(xrange(), yrange()).spawn(Agent{Carnivore});
        }
    }

    bool n1p = false;
    bool n2p = false;
    bool n3p = false;
    bool n4p = false;

    void postprocessing(Grid & /*grid*/) {

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num1)) {
            if (n1p == false) {
                show_vegetation = !show_vegetation;
                n1p = true;
            }
        }
        else if (n1p == true)
            n1p = false;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num2)) {
            if (n2p == false) {
                show_herbivores = !show_herbivores;
                n2p = true;
            }
        }
        else if (n2p == true)
            n2p = false;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num3)) {
            if (n3p == false) {
                show_carnivores = !show_carnivores;
                n3p = true;
            }
        }
        else if (n3p == true)
            n3p = false;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num4)) {
            if (n4p == false) {
                show_vegetation = true;
                show_herbivores = true;
                show_carnivores = true;
                n4p = true;
            }
        }
        else if (n4p == true)
            n4p = false;
    }
};

int main() {
    CASE::Dynamic<Config>();
}

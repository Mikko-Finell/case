#include <iostream>

#include <SFML/Graphics/Color.hpp>

#include "../quad.hpp"
#include "../index.hpp"
#include "../random.hpp"
#include "../grid.hpp"
#include "../cell.hpp"
#include "../simulator.hpp"

#define COLUMNS 256
#define ROWS 256
#define CELL_SIZE 2
#define ORG_NUM_GENES 7
#define TOT_NUM_GENES 11
#define MUTATION_RATE 2
#define MAX_HP 1000

template <int MIN, int MAX>
int clamp(const int value) {
    return value < MIN ? MIN : value > MAX ? MAX : value;
}

struct Color {
    int r=0, g=0, b=0;
    void add(const Color & other) {
        r = clamp<0, 255>(r + other.r);
        g = clamp<0, 255>(g + other.g);
        b = clamp<0, 255>(b + other.b);
    }
    int diff(const Color & other) const {
        const auto rd = std::abs(r - other.r) / 255.0;
        const auto gd = std::abs(g - other.g) / 255.0;
        const auto bd = std::abs(b - other.b) / 255.0;
        return (rd + gd + bd) / 300.0;
    }
    int & operator[](const int index) {
        if (index == 0)      return r;
        else if (index == 1) return g;
        else                 return b;
    }
};

class Organism;
static CASE::RDist<0,50> rcolor;

class Gene {
    virtual void _act(Organism *) {}
    virtual void _act(Organism *, Organism *) {}

public:
    Color color;

    Gene() {
        static CASE::RDist<0,2> rgb;
        color[rgb()] = 50;
        color[rgb()] = 20;
    }
    void act(Organism * self, Organism * other = nullptr) {
        if (other == nullptr)
            _act(self);
        else
            _act(self, other);
    }
    virtual void print() const {}
    virtual ~Gene() {}
};

std::array<Gene *, TOT_NUM_GENES> GENEPOOL;

class Organism {
    bool alive = false;
    std::array<Gene *, ORG_NUM_GENES> gene;

public:
    int age = 0;
    int hp = MAX_HP;
    using Cell = CASE::ZCell<Organism, 1>;
    int z = 0;
    Cell * cell = nullptr;

    void update();
    Organism();
    Organism(const Organism * parent);
    void draw(const int x, const int y, std::vector<sf::Vertex> & vs) {
        const auto mycolor = this->color();
        static const int pad = 0;
        auto size = vs.size();
        vs.resize(size + 4);
        CASE::quad(pad + x * CELL_SIZE, pad + y * CELL_SIZE,
                CELL_SIZE - 2 * pad, CELL_SIZE - 2 * pad, 
                mycolor.r, mycolor.g, mycolor.b, &vs[size]);
    }
    bool active() const { return alive; }
    void activate() { alive = true; }
    void deactivate() { alive = false; }

    Color color() const {
        Color mycolor{0, 0, 0};
        for (auto g : gene)
            mycolor.add(g->color);
        return mycolor;
    }
};

static CASE::RDist<-1, 1> uv;
static CASE::RDist<0,100> r_100;
static CASE::RDist<0,1000> r_1000;
static CASE::RDist<0, TOT_NUM_GENES - 1> gene_select;

Organism::Organism() {
    gene.fill(nullptr);
    for (auto i = 0; i < ORG_NUM_GENES; i++)
        gene.at(i) = GENEPOOL.at(gene_select());
}

Organism::Organism(const Organism * parent) {
    for (auto i = 0; i < ORG_NUM_GENES; i++) {
        if (r_1000() < MUTATION_RATE)
            gene.at(i) = GENEPOOL.at(gene_select());
        else
            gene.at(i) = parent->gene.at(i);
    }
}

void Organism::update() {
    if (hp <= 0 || ++age > 100) {
        deactivate();
        return;
    }

    static CASE::RDist<0, ORG_NUM_GENES - 1> rgene;
    gene.at(rgene())->act(this);

    static const std::array<int, 4> compass{ {0,1,2,3} };
    static const int dir[4][2] = { {-1,0},{1,0}, {0,-1},{0,1} };
    static CASE::Uniform r;

    const auto mycolor = this->color();

    auto neighbors = cell->neighbors();
    auto popcount = 0;
    for (const auto i : r.shuffled(compass)) {
        const auto x = dir[i][0], y = dir[i][1];
        auto neighbor = neighbors(x, y)[z];
        if (neighbor != nullptr) {
            popcount++;
            if (mycolor.diff(neighbor->color()) == 0)
                gene.at(1)->act(this, neighbor);
            else
                gene.at(2)->act(this, neighbor);
        }
        else
            gene.at(3)->act(this);
    }

    if (popcount < 3)
        gene.at(4)->act(this);
    else if (popcount > 3)
        gene.at(5)->act(this);
    else
        gene.at(6)->act(this);
}

class Gene0 : public Gene { // reproduce
    void _act(Organism * self) override {
        if (r_100() < 5) {
            static const int dir[4][2] = { {-1,0},{1,0}, {0,-1},{0,1} };
            static CASE::RDist<0,3> r;
            const auto i = r();
            auto neighbors = self->cell->neighbors();
            const auto x = dir[i][0], y = dir[i][1];
            if (neighbors(x, y).spawn(Organism{self}) != nullptr)
                self->hp -= MAX_HP * 0.25;
        }
    }
};

class Gene1_up : public Gene {
    void _act(Organism * self) override {
        auto neighbors = self->cell->neighbors();
        neighbors(0, -1).insert(self);
    }
    void _act(Organism * self, Organism *) override {
        auto neighbors = self->cell->neighbors();
        neighbors(0, 1).insert(self);
    }
};
class Gene1_down : public Gene { // move down
    void _act(Organism * self) override {
        auto neighbors = self->cell->neighbors();
        neighbors(0, 1).insert(self);
    }
    void _act(Organism * self, Organism *) override {
        auto neighbors = self->cell->neighbors();
        neighbors(0, -1).insert(self);
    }
};
class Gene1_left : public Gene { // move left
    void _act(Organism * self) override {
        auto neighbors = self->cell->neighbors();
        neighbors(-1, 0).insert(self);
    }
    void _act(Organism * self, Organism *) override {
        auto neighbors = self->cell->neighbors();
        neighbors(1, 0).insert(self);
    }
};
class Gene1_right : public Gene { // move right
    void _act(Organism * self) override {
        auto neighbors = self->cell->neighbors();
        neighbors(1, 0).insert(self);
    }
    void _act(Organism * self, Organism *) override {
        auto neighbors = self->cell->neighbors();
        neighbors(-1, 0).insert(self);
    }
};

class Gene2 : public Gene { // kill
    void _act(Organism * self, Organism * other) override {
        other->hp = 0;
    }
};

class Gene3 : public Gene { // steal hp
    void _act(Organism * self) override {
        self->hp -= 10;
    }
    void _act(Organism * self, Organism * other) override {
        self->hp = clamp<0, MAX_HP>(self->hp + 0.5 * other->hp);
        other->hp *= 0.5;
    }
};

class Gene4 : public Gene { // average hp
    void _act(Organism * self, Organism * other) override {
        const auto hp = (self->hp + other->hp) / 2.0;
        self->hp = hp;
        other->hp = hp;
    }
};

class Gene5 : public Gene { // give hp
    void _act(Organism * self) override {
        const auto hp = 0.1 * self->hp;
        self->hp = clamp<0, MAX_HP>(self->hp + hp);
    }
    void _act(Organism * self, Organism * other) override {
        const auto hp = 0.25 * self->hp;
        self->hp -= hp;
        other->hp = clamp<0, MAX_HP>(other->hp + hp);
    }
};

class Gene6 : public Gene { // damage
    void _act(Organism * self) override {
        self->hp -= MAX_HP * 0.1;
    }
    void _act(Organism * self, Organism * other) override {
        other->hp -= MAX_HP * 0.2;
    }
};

class Gene7 : public Gene { // die
    void _act(Organism * self) override {
        self->hp = 0;
    }
    void _act(Organism * self, Organism *) override {
    }
};

struct Config {
    using Agent = Organism;
    using Cell = Agent::Cell;
    using Grid = CASE::Grid<Cell>;

    static constexpr int columns = COLUMNS;
    static constexpr int rows = ROWS;
    static constexpr int cell_size = CELL_SIZE;
    static constexpr int subset = columns * rows * Cell::depth;

    const double framerate = 60;
    const char* title = "Color Evolution";
    const sf::Color bgcolor{0,0,0};

    void init(Grid & grid, CASE::AgentManager<Agent> & manager) {
        grid.clear();
        manager.clear();

        GENEPOOL.fill(nullptr);
        GENEPOOL.at(0) = new Gene0;
        GENEPOOL.at(1) = new Gene1_up;
        GENEPOOL.at(2) = new Gene1_down;
        GENEPOOL.at(3) = new Gene1_left;
        GENEPOOL.at(4) = new Gene1_right;
        GENEPOOL.at(5) = new Gene2;
        GENEPOOL.at(6) = new Gene3;
        GENEPOOL.at(7) = new Gene4;
        GENEPOOL.at(8) = new Gene5;
        GENEPOOL.at(9) = new Gene6;
        GENEPOOL.at(10) = new Gene7;

        CASE::RDist<0, columns - 1> xrange;
        CASE::RDist<0, rows - 1> yrange;

        for (auto i = 0; i < 32; i++) {
            grid(xrange(), yrange()).spawn(Agent{});
        }
    }

    void postprocessing(Grid & /*grid*/) {
    }
};

int main() {
    CASE::simulator::Dynamic<Config>();
}


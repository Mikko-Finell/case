#include "../random.hpp"
#include "../neighbors.hpp"
#include "../quad.hpp"
#include "../grid.hpp"
#include "../static_sim.hpp"

#define COLUMNS 256
#define ROWS 256
#define CELL_SIZE 2

class Brian {
    int x, y;
    enum State { On, Dying, Dead };
    State state;

public:
    void activate() { state = On; }
    int index = 0;

    Brian(int _x = 0, int _y = 0) : x(CELL_SIZE * _x), y(CELL_SIZE * _y)
    {
        state = Dead;
    }

    void update(Brian & next) const {
        auto neighbors = CASE::CAdjacent<Brian>{this};
        static const int range[3] = {-1, 0, 1};
        auto count = 0;
        for (const auto y : range) {
            for (const auto x : range) {
                if (x == 0 && y == 0)
                    continue;
                if (neighbors(x, y).state == On)
                    count++;
            }
        }
        if (state == Dead) {
            if (count == 2)
                next.state = On;
        }
        else if (state == On)
            next.state = Dying;

        else if (state == Dying)
            next.state = Dead;
    }

    void draw(sf::Vertex * vs) const {
        static constexpr int pad = 0;
        if (state == On)
            CASE::quad(x, y, CELL_SIZE-pad, CELL_SIZE-pad,
                    0, 0, 0, vs);
        else if (state == Dying)
            CASE::quad(x, y, CELL_SIZE-pad, CELL_SIZE-pad,
                    100, 100, 255, vs);
        else
            CASE::quad(x, y, CELL_SIZE-pad, CELL_SIZE-pad,
                    255, 255, 255, vs);
    }
};

struct BriansBrain {
    using Agent = Brian;

    static constexpr int columns = COLUMNS;
    static constexpr int rows = ROWS;
    static constexpr int cell_size = CELL_SIZE;
    static constexpr int subset = columns * rows;
    double framerate = 60.0;
    const char* title = "Brians Brain";
    const sf::Color bgcolor = sf::Color::White;

    void init(Brian * agents) {
        int index = 0;
        for (auto y = 0; y < ROWS; y++) {
            for (auto x = 0; x < COLUMNS; x++) {
                auto & agent = agents[CASE::index(x, y, COLUMNS)];
                agent = Brian{x, y};
                agent.index = index++;
            }
        }
        CASE::Gaussian<(columns+rows)/4, 25> random;
        for (auto i = 0; i < 250; i++) {
            const auto x = CASE::wrap(random(), columns),
                       y = CASE::wrap(random(), rows);

            auto & agent = agents[CASE::index(x, y, columns)];
            agent.activate();
        }
    }

    void postprocessing(Agent *) {}
};

int main() {
    CASE::Static<BriansBrain>();
}

#include <SFML/Graphics.hpp>
#include <cpptest.hpp>
#include "../array_buffer.hpp"
#include "../graphics.hpp"
#include "../update.hpp"

namespace t_graphics {

void quad(sf::Vector2f pos, sf::Vector2f size, sf::Color color,
          sf::Vertex * vertices)
{
    vertices[0] = sf::Vertex{pos, color};
    vertices[1] = sf::Vertex{pos + sf::Vector2f{size.x,0}, color};
    vertices[2] = sf::Vertex{pos + size, color};
    vertices[3] = sf::Vertex{pos + sf::Vector2f{0, size.y}, color};
}

class Agent {
    sf::Vector2f size;
    sf::Color color;
    int dirx = 10;
    int diry = 5;
public:
    int seed = 1;
    sf::Vector2f position;
    void init() {
        std::mt19937 rng(seed);
        std::uniform_int_distribution<sf::Uint8> x(150, 255);
        color = sf::Color{x(rng), x(rng), x(rng)};
        size = sf::Vector2f(x(rng), x(rng));
    }
    Agent update(Agent copy) {
        if ((copy.position.x > 1600 - size.x && copy.dirx > 0)
        || (copy.position.x < 0 && copy.dirx < 0))
            copy.dirx *= -1;
        copy.position.x += copy.dirx;

        if ((copy.position.y > 800 - size.y && copy.diry > 0)
        || (copy.position.y < 0 && copy.diry < 0))
            copy.diry *= -1;
        copy.position.y += copy.diry;

        return copy;
    }
    void draw(sf::Vertex * vertices) const {
        quad(position, size, color, vertices);
    }
};

bool graphics_01() {
    sf::RenderWindow w{sf::VideoMode{1600, 800}, "test 01"};
    w.setVerticalSyncEnabled(true);
    std::vector<sf::Vertex> vertices;

    const auto seed = std::random_device()();
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> posx(-200, 1600), posy(-100, 800);

    constexpr auto size = 1000;
    Agent a[size], b[size];
    vertices.resize(size * 4);
    CASE::Update<Agent> update;
    CASE::Graphics<Agent> graphics;
    CASE::ArrayBuffer<Agent> world{a, b};

    update.set_trigger(CASE::Trigger{0, 0.2, 1});

    for (auto & agent : b) {
        agent.position = sf::Vector2f(posx(rng), posy(rng));
        agent.seed = posx(rng);
        agent.init();
    }

    auto x = 10;
    vertices.resize(x * 4);

    auto subset = 1;

    while (w.isOpen()) {
        sf::Event e;
        while (w.pollEvent(e)) {
            if (e.type == sf::Event::KeyPressed) {
                if (e.key.code == sf::Keyboard::Q) {
                    w.close();
                }
                if (e.key.code == sf::Keyboard::Up)
                    x++;
                else if (e.key.code == sf::Keyboard::Down)
                    x--;
                if (e.key.code == sf::Keyboard::Left)
                    subset--;
                else if (e.key.code == sf::Keyboard::Right)
                    subset++;
                //vertices.clear();
                vertices.resize(x * 4);
                if (x > size)
                    x = size;
                if (x < 0)
                    x = 0;
            }
            if (e.type == sf::Event::Closed)
                w.close();

        }

        graphics.wait();
        update.wait();

        w.setTitle("Updating " + std::to_string(subset) 
                + " of " + std::to_string(x));

        w.clear(sf::Color::White);
        w.draw(vertices.data(), vertices.size(), sf::Quads);
        w.display();

        world.flip();

        update.launch(world.current(), world.next(), x, subset);
        graphics.draw(world.current(), vertices.data(), x);
    }

    update.wait();
    graphics.wait();

    return false;
}

void run() {
    cpptest::Module test{"graphics"};
    test.fn("moving squares", graphics_01);
}

}

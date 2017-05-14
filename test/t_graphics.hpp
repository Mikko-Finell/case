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
public:
    int seed = 1;
    int dirx = 10;
    int diry = 5;
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

    const auto seed = std::random_device()();
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> posx(-200, 1600), posy(-100, 800);

    constexpr auto size = 19 * 10;
    Agent a[size], b[size];
    CASE::Update<Agent> update;
    CASE::Graphics<Agent> graphics{w};
    CASE::ArrayBuffer<Agent> world{a, b};

    for (auto & agent : b) {
        agent.position = sf::Vector2f(posx(rng), posy(rng));
        agent.dirx = agent.dirx * (posy(rng) > 450 ? -1 : 1);
        agent.diry = agent.diry * (posy(rng) > 450 ? -1 : 1);
        agent.seed = posx(rng);
        agent.init();
    }

    while (w.isOpen()) {
        sf::Event e;
        while (w.pollEvent(e)) {
            if (e.type == sf::Event::KeyPressed) {
                if (e.key.code == sf::Keyboard::Q)
                    w.close();
            }
            if (e.type == sf::Event::Closed)
                w.close();
        }

        graphics.wait();
        update.wait();
        world.flip();

        update.launch(world.current(), world.next(), size);
        graphics.draw(world.current(), size);

        graphics.clear_screen();
        graphics.display();
    }

    update.wait();
    graphics.wait();

    return false;
}

void run() {
    //cpptest::Module test{"graphics"};
    //test.fn("moving squares", graphics_01);
    graphics_01();
}

}

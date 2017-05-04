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
    int seed = 0;
public:
    sf::Vector2f position;
    Agent() {
        seed = std::random_device()();
        std::mt19937 rng(seed);
        std::uniform_int_distribution<sf::Uint8> x(150, 255);
        color = sf::Color{x(rng), x(rng), x(rng)};
        size = sf::Vector2f(x(rng), x(rng));
    }
    Agent update(Agent copy) {
        std::mt19937 rng(seed);
        std::uniform_int_distribution<int> move(-1, 1);
        copy.position += sf::Vector2f(move(rng), move(rng));
        copy.seed++;
        return copy;
    }
    void draw(sf::Vertex * vertices) const {
        quad(position, size, color, vertices);
    }
};

bool graphics_01() {
    sf::RenderWindow w{sf::VideoMode{1600, 800}, "test 01"};
    std::vector<sf::Vertex> vertices;

    const auto seed = std::random_device()();
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> posx(0, 1600), posy(0, 800);

    constexpr auto size = 500;
    Agent a[size], b[size];
    vertices.resize(size * 4);
    CASE::Update<Agent> update;
    CASE::Graphics<Agent> graphics;
    CASE::ArrayBuffer<Agent> world{a, b};

    update.set_trigger(CASE::Trigger{16.7, 0.2, 1});

    for (auto & agent : b)
        agent.position = sf::Vector2f(posx(rng), posy(rng));


    while (w.isOpen()) {
        sf::Event e;
        while (w.pollEvent(e)) {
            if (e.type == sf::Event::KeyPressed)
                w.close();
            if (e.type == sf::Event::Closed)
                w.close();
        }

        update.wait();

        world.flip();

        update.launch(world.current(), world.next(), size);
        graphics.draw(world.current(), vertices.data(), size);

        w.clear(sf::Color::White);
        graphics.wait();
        w.draw(vertices.data(), vertices.size(), sf::Quads);
        w.display();
    }

    update.wait();
    //graphics.wait();

    return false;
}

void run() {
    cpptest::Module test{"graphics"};
    test.fn("moving squares", graphics_01);
}

}

#ifndef CASE_GRAPHM
#define CASE_GRAPHM

#include <cassert>
#include <SFML/Graphics.hpp>

#include "map.hpp"
#include "job.hpp"
#include "array_buffer.hpp"

namespace CASE {
namespace graphics {

namespace dynamic {
template<class Cell>
class Job : public job::Base {

    void execute() override {
        for (auto i = 0; i < array_size; i++)
            cells[i].draw(*vertices);
    }

    const Cell * cells = nullptr;
    std::vector<sf::Vertex> * vertices = nullptr;
    volatile int array_size = 0;

public:
    Job(const int, const std::size_t)
        : job::Base(0, 1)
    {}

    void upload(const Cell * _c, std::vector<sf::Vertex> & vs, const int size) {
        cells = _c;
        vertices = &vs;
        array_size = size;
    }
};

template<class Cell>
class DoubleBuffer : private map::Parallel<Job<Cell>> {

    enum class Access { Open, Closed };
    Access access = Access::Closed;

    sf::RenderWindow * window = nullptr;
    std::vector<sf::Vertex> vertices[2];
    ArrayBuffer<std::vector<sf::Vertex>> vs{&vertices[0], &vertices[1]};

    Job<Cell> job{0, 1};

    void wait() {
        if (access == Access::Open)
            return;
        job.wait();
        access = Access::Open;
    }

public:
    DoubleBuffer(sf::RenderWindow & w) : window(&w)
    {
        job.thread = std::thread{[this]{ job.run(); }};
        wait();
    }

    ~DoubleBuffer() {
        wait();

        if (job.thread.joinable()) {
            job.terminate();
            job.upload(nullptr, *vs.current(), 0);
            job.launch();
            job.thread.join();
        }
    }

    void draw(const Cell * cells, const int size) {
        wait();

        vs.flip(); // swap buffers
        vs.next()->clear();

        job.upload(cells, *vs.next(), size);
        job.launch();

        access = Access::Closed;

        auto & current = *vs.current();
        window->draw(current.data(), current.size(), sf::Quads);
        window->display();
    }

    void clear(const sf::Color color = sf::Color::White) {
        window->clear(color);
    }

    void display() { return; }

};

template<class Cell>
class SingleBuffer {
    sf::RenderWindow * window = nullptr;
    std::vector<sf::Vertex> vertices;

public:
    SingleBuffer(sf::RenderWindow & w) : window(&w)
    {
    }

    void draw(const Cell * cells, const int size) {
        assert(window != nullptr);
        assert(cells != nullptr);
        assert(size >= 0);

        vertices.clear();

        for (auto i = 0; i < size; i++)
            cells[i].draw(vertices);
    }

    void clear(const sf::Color color = sf::Color::White) {
        window->clear(color);
    }

    void display() {
        window->draw(vertices.data(), vertices.size(), sf::Quads);
        window->display();
    }
};

} // dynamic
} // graphics

} // CASE

#endif // GRAPHM

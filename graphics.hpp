#ifndef CASE_GRAPHM
#define CASE_GRAPHM

#include <cassert>
#include <SFML/Graphics.hpp>

#include "map.hpp"
#include "job.hpp"
#include "array_buffer.hpp"

namespace CASE {
namespace graphics {

namespace static_vs {
template<class T, int VS_PER_OBJ>
class Job : public job::Base {
    static_assert(VS_PER_OBJ % 4 == 0, "vs per object must be multiple of 4");

    void execute() override {
        for (auto i = nth; i < array_size; i += n_threads)
            objects[i].draw(vertices + i * VS_PER_OBJ);
    }

    const T * objects = nullptr;
    sf::Vertex * vertices = nullptr;
    volatile int array_size = 0;

public:
    Job(const int nth, const std::size_t n_threads)
        : job::Base(nth, n_threads)
    {}

    void upload(const T * objs, sf::Vertex * vs, const int count) {
        objects = objs;
        vertices = vs;
        array_size = count;
    }
};
} // static_vs

template<class T, int VS_PER_OBJ>
class Parallel : private map::Parallel<static_vs::Job<T, VS_PER_OBJ>> {
    static_assert(VS_PER_OBJ % 4 == 0, "vs per object must be multiple of 4");

    enum class Access { Open, Closed };
    Access access = Access::Closed;

    sf::RenderWindow * window = nullptr;
    std::vector<sf::Vertex> vertices[2];
    ArrayBuffer<std::vector<sf::Vertex>> vs{&vertices[0], &vertices[1]};

public:
    Parallel(sf::RenderWindow & w) : window(&w)
    {
        map::Parallel<static_vs::Job<T, VS_PER_OBJ>>::init();
        wait();
    }

    ~Parallel() {
        terminate();
    }

    void wait() {
        if (access == Access::Open)
            return;

        map::Parallel<static_vs::Job<T, VS_PER_OBJ>>::wait();

        access = Access::Open;
    }

    Parallel & draw(const T * objects, const int size) {
        assert(window != nullptr);
        assert(objects != nullptr);
        assert(size >= 0);

        wait();

        vs.flip(); // swap buffers
        vs.next()->resize(size * VS_PER_OBJ);

        for (auto & job : this->jobs) {
            job.upload(objects, vs.next()->data(), size);
            job.launch();
        }

        return *this;
    }

    void clear(const sf::Color color = sf::Color::White) {
        window->clear(color);
    }

    void display() {
        auto & current = *vs.current();
        window->draw(current.data(), current.size(), sf::Quads);
        window->display();
    }

    void terminate() {
        for (auto & job : this->jobs) {
            job.terminate();
            job.upload(nullptr, nullptr, 0);
            job.launch();
            job.thread.join();
        }
        this->jobs.clear();
    }
};

namespace dynamic {
template<class T>
class Job : public job::Base {

    void execute() override {
        for (auto i = 0; i < array_size; i++)
            objects[i].draw(*vertices);
    }

    const T * objects = nullptr;
    std::vector<sf::Vertex> * vertices = nullptr;
    volatile int array_size = 0;

public:
    Job(const int nth, const std::size_t n_threads)
        : job::Base(0, 1)
    {}

    void upload(const T * objs, std::vector<sf::Vertex> & vs, const int count) {
        objects = objs;
        vertices = &vs;
        array_size = count;
    }
};

template<class T>
class DoubleBuffer : private map::Parallel<Job<T>> {

    enum class Access { Open, Closed };
    Access access = Access::Closed;

    sf::RenderWindow * window = nullptr;
    std::vector<sf::Vertex> vertices[2];
    ArrayBuffer<std::vector<sf::Vertex>> vs{&vertices[0], &vertices[1]};

    Job<T> job{0, 1};

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

    void draw(const T * objects, const int size) {
        wait();

        vs.flip(); // swap buffers
        vs.next()->clear();

        job.upload(objects, *vs.next(), size);
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

template<class T>
class SingleBuffer {
    sf::RenderWindow * window = nullptr;
    std::vector<sf::Vertex> vertices;

public:
    SingleBuffer(sf::RenderWindow & w) : window(&w)
    {
    }

    void draw(const T * objects, const int size) {
        assert(window != nullptr);
        assert(objects != nullptr);
        assert(size >= 0);

        vertices.clear();

        for (auto i = 0; i < size; i++)
            objects[i].draw(vertices);
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

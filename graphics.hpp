#ifndef CASE_GRAPHM
#define CASE_GRAPHM

#include <SFML/Graphics.hpp>

#include "map.hpp"
#include "job.hpp"
#include "trigger.hpp"

namespace CASE {
namespace graphics {

template<class T>
class Serial : public map::Serial {
public:
    void draw(const T * objects, sf::Vertex * vs, const int size) {
        assert(objects != nullptr);
        assert(vs != nullptr);
        assert(size >= 0);

        timer.start();
        for (int i = 0; i < size; i++)
            objects[i].draw(vs + i * 4);
        timer.stop();
    }
};

template<class T>
class Job : public job::Base {
    void execute() override {
        const int count = array_size;
        for (auto i = nth; i < count; i += n_threads)
            objects[i].draw(vertices + i * 4);
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


template<class T>
class Parallel : public map::Parallel<Job<T>> {
public:
    void draw(const T * objects, sf::Vertex * vs, const int size) {
        assert(objects != nullptr);
        assert(vs != nullptr);
        assert(size >= 0);

        for (auto & job : this->jobs) {
            job.upload(objects, vs, size);
            job.launch();
        }
    }

    void terminate() override {
        for (auto & job : this->jobs) {
            job.terminate();
            job.upload(nullptr, nullptr, 0);
            job.launch();
            job.thread.join();
        }
        this->jobs.clear();
    }
};

template<class T>
class Manager : public map::Manager<Serial<T>, Parallel<T>> {
    sf::RenderWindow * window = nullptr;
    std::vector<sf::Vertex> vertices[2];
    ArrayBuffer<std::vector<sf::Vertex>> vs{&vertices[0], &vertices[1]};

public:
    Manager(sf::RenderWindow & w)
        : map::Manager<Serial<T>, Parallel<T>>::Manager{},
        window(&w)
    {
    }

    Manager & draw(const T * objects, const int size) {
        this->prelaunch();
        assert(window != nullptr);

        vs.flip(); // swap buffers
        vs.next()->resize(size * 4);
        if (this->strategy == map::Strategy::Serial)
            this->serial.draw(objects, vs.next()->data(), size);
        else
            this->parallel.draw(objects, vs.next()->data(), size);

        this->postlaunch();
        return *this;
    }

    void clear(const sf::Color color = sf::Color::White) {
        window->clear(color);
    }

    void display() {
        window->draw(vs.current()->data(), vs.current()->size(), sf::Quads);
        window->display();
    }
};

} // graphics

template<class T>
using Graphics = graphics::Manager<T>;

} // CASE

#endif // GRAPHM

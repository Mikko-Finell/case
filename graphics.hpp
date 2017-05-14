#ifndef CASE_GRAPHM
#define CASE_GRAPHM

#include <SFML/Graphics.hpp>

#include "map.hpp"
#include "job.hpp"
#include "trigger.hpp"

namespace CASE {
namespace graphics {

template<class T>
class Job : public job::Base {
    void execute() override {
        for (auto i = nth; i < array_size; i += n_threads)
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
class Parallel : private map::Parallel<Job<T>> {
    enum class Access { Open, Closed };
    Access access = Access::Closed;

    sf::RenderWindow * window = nullptr;
    std::vector<sf::Vertex> vertices[2];
    ArrayBuffer<std::vector<sf::Vertex>> vs{&vertices[0], &vertices[1]};

public:
    Parallel(sf::RenderWindow & w) : window(&w)
    {
        map::Parallel<Job<T>>::init();
        wait();
    }

    ~Parallel() {
        terminate();
    }

    void wait() {
        if (access == Access::Open)
            return;

        map::Parallel<Job<T>>::wait();

        access = Access::Open;
    }

    Parallel & draw(const T * objects, const int size) {
        wait();

        vs.flip(); // swap buffers
        vs.next()->resize(size * 4);

        assert(window != nullptr);
        assert(objects != nullptr);
        assert(size >= 0);

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

} // graphics

template<class T>
using Graphics = graphics::Parallel<T>;

} // CASE

#endif // GRAPHM

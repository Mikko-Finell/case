#ifndef CASE_GRAPHM
#define CASE_GRAPHM

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
        const auto count = array_size;
        for (auto n = nth; n < count; n += n_threads)
            objects[n].draw(vertices + n * 4);
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
public:
    using map::Manager<Serial<T>, Parallel<T>>::Manager;

    Manager & draw(const T * objects, sf::Vertex * vs, const int size) {
        assert(this->access == map::Access::Open);

        if (this->strategy == map::Strategy::Serial)
            this->serial.draw(objects, vs, size);
        else
            this->parallel.draw(objects, vs, size);

        this->access = map::Access::Closed;
        return *this;
    }
};

} // graphics

template<class T>
using Graphics = graphics::Manager<T>;

} // CASE

#endif // GRAPHM

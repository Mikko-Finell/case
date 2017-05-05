#ifndef CASE_UPDATEM
#define CASE_UPDATEM

#include "job.hpp"
#include "map.hpp"
#include "trigger.hpp"

namespace CASE {
namespace update {

template<class T>
class Serial : public map::Serial {
public:
    void launch(T * current, T * next, const int size) {
        assert(current != nullptr);
        assert(next != nullptr);
        assert(current != next);
        assert(size >= 0);

        if (current < next)
            assert(current + size <= next);
        else
            assert(current >= next + size);

        this->timer.start();
        for (int i = 0; i < size; i++)
            next[i] = current[i].update(current[i]);
        this->timer.stop();
    }
};

template<class T>
class Job : public job::Base {
    void execute() override {
        const auto count = array_size;
        for (auto n = nth; n < count; n += n_threads)
            next[n] = current[n].update(current[n]);
    }

    T * current = nullptr;
    T * next = nullptr;
    volatile int array_size = 0;

public:
    Job(const int nth, const std::size_t n_threads)
        : job::Base(nth, n_threads)
    {}

    void upload(T * first, T * second, const int count) {
        current = first;
        next = second;
        array_size = count;
    }
};

template<class T>
class Parallel : public map::Parallel<Job<T>> {
public:
    void launch(T * current, T * next, const int size) {
        assert(current != nullptr);
        assert(next != nullptr);
        assert(current != next);
        assert(size >= 0);

        if (current < next)
            assert(current + size <= next);
        else
            assert(current >= next + size);

        for (auto & job : this->jobs) {
            job.upload(current, next, size);
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

    Manager & launch(T * current, T * next, const int size) {
        this->prelaunch();

        if (this->strategy == map::Strategy::Serial)
            this->serial.launch(current, next, size);
        else
            this->parallel.launch(current, next, size);

        this->postlaunch();
        return *this;
    }
};

} // update

template<class T>
using Update = update::Manager<T>;

} // CASE

#endif // UPDATEM

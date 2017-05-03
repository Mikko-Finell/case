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
        assert(current != next);
        assert(current != nullptr);
        assert(next != nullptr);
        assert(size > 0);
        assert(size % 2 == 0);
        if (current < next)
            assert(current + size <= next);
        else
            assert(current > next + size);

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

    T * current;
    T * next;
    volatile int array_size;

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
        for (auto & job : this->jobs) {
            job.upload(current, next, size);
            job.launch();
        }
    }

    void terminate() {
        if (!this->jobs.empty()) {
            for (auto & job : this->jobs)
                job.terminate();

            launch(nullptr, nullptr, 0);
            for (auto & job : this->jobs) {
                if (job.thread.joinable())
                    job.thread.join();
            }
            this->jobs.clear();
        }
    }
};

template<class T>
class Manager : private map::Base {
    Serial<T> serial;
    Parallel<T> parallel;
    map::Base & strategy = serial;
    CASE::Trigger trigger;

public:
    Manager(const CASE::Trigger & tr = CASE::Trigger{1000.0/60.0, 0.2})
        : trigger(tr)
    {
    }
    // init
    // wait
    // launch
    // terminate
};

} // update
} // CASE

#endif // UPDATEM

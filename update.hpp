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
class Manager {
    Serial<T> serial;
    Parallel<T> parallel;
    CASE::Trigger trigger;

    enum class Strategy { Serial, Parallel };
    Strategy strategy = Strategy::Serial;

    enum class Access { Allowed, Restricted };
    Access access = Access::Restricted;

public:
    Manager(CASE::Trigger && tr = CASE::Trigger{1000.0/60.0, 0.2})
        : trigger(tr)
    {
        serial.init();
        parallel.init();
        serial.wait();
        parallel.wait();
        access = Access::Allowed;
    }

    ~Manager() {
        serial.terminate();
        parallel.terminate();
    }

    void set_trigger(CASE::Trigger && tr) {
        trigger = tr;
    }

    Manager & wait() {
        if (access == Access::Allowed)
            return *this;

        if (strategy == Strategy::Serial) {
            serial.wait();
            if (trigger.update(serial.job_duration())) {
                strategy = Strategy::Parallel;
            }
        }
        else {
            parallel.wait();
            if (!trigger.update(parallel.job_duration())) {
                strategy = Strategy::Serial;
            }
        }

        access = Access::Allowed;
        return *this;
    }

    Manager & launch(T * current, T * next, const int size) {
        assert(access == Access::Allowed);

        if (strategy == Strategy::Serial)
            serial.launch(current, next, size);
        else
            parallel.launch(current, next, size);

        access = Access::Restricted;
        return *this;
    }
};

} // update
} // CASE

#endif // UPDATEM

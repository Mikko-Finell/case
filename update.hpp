#ifndef CASE_UPDATEM
#define CASE_UPDATEM

//#include <cstring>
#include <numeric>
#include <vector>

#include "job.hpp"
#include "map.hpp"
#include "trigger.hpp"

namespace CASE {
namespace update {
template<class T>
class Serial : public map::Serial {
    std::random_device rd;
    std::mt19937 rng{rd()};
    std::vector<int> indices;

public:
    void launch(T * current, T * next, const int size, const int subset) {
        assert(current != nullptr);
        assert(next != nullptr);
        assert(current != next);
        assert(size >= 0);
        assert(subset >= 0);

        if (current < next)
            assert(current + size <= next);
        else
            assert(current >= next + size);

        this->timer.start();

        // TODO check whether memmove is faster than current mechanism
        //std::memmove(next, current, size); 
        for (auto i = 0; i < size; i++)
            next[i] = current[i];

        if (subset < size) {
            indices.resize(size);
            std::iota(indices.begin(), indices.end(), 0);
            std::shuffle(indices.begin(), indices.end(), rng);

            for (int i = 0; i < subset; i++) {
                const auto index = indices[i];
                current[index].update(next[index]);
            }
        }
        else {
            for (int i = 0; i < size; i++)
                current[i].update(next[i]);
        }

        this->timer.reset();
    }

    void launch(T * current, T * next, const int size) {
        launch(current, next, size, size);
    }
};

template<class T>
class Job : public job::Base {
    std::random_device rd;
    std::mt19937 rng{rd()};
    std::vector<int> indices;

    void execute() override {
        for (auto i = nth; i < array_size; i += n_threads) {
            indices.push_back(i);
            next[i] = current[i];
        }
        if (subset < array_size) {
            std::shuffle(indices.begin(), indices.end(), rng);

            for (auto i = nth; i < subset; i += n_threads) {
                const auto index = indices.back();
                indices.pop_back();
                current[index].update(next[index]);
            }
            for (const auto i : indices)
                next[i] = current[i];
        }
        else {
            for (auto i = nth; i < array_size; i += n_threads)
                current[i].update(next[i]);
        }
        indices.clear();
    }

    T * current = nullptr;
    T * next = nullptr;
    volatile int array_size = 0;
    volatile int subset = 0;

public:
    Job(const int nth, const std::size_t n_threads)
        : job::Base(nth, n_threads)
    {}

    void upload(T * first, T * second, const int count, const int ss) {
        current = first;
        next = second;
        array_size = count;
        subset = ss;
    }
};

template<class T>
class Parallel : public map::Parallel<Job<T>> {
public:
    void launch(T * current, T * next, const int size, const int subset) {
        assert(current != nullptr);
        assert(next != nullptr);
        assert(current != next);
        assert(size >= 0);
        assert(subset >= 0);

        if (current < next)
            assert(current + size <= next);
        else
            assert(current >= next + size);

        for (auto & job : this->jobs) {
            job.upload(current, next, size, subset);
            job.launch();
        }
    }

    void launch(T * current, T * next, const int size) {
        // TODO check whether memmove is faster than current mechanism
        //std::memmove(next, current, size); 
        launch(current, next, size, size);
    }

    void terminate() override {
        for (auto & job : this->jobs) {
            job.terminate();
            job.upload(nullptr, nullptr, 0, 0);
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

    Manager & launch(T * current, T * next, const int size, int subset)
    {
        this->prelaunch();

        if (this->strategy == map::Strategy::Serial)
            this->serial.launch(current, next, size, subset);
        else
            this->parallel.launch(current, next, size, subset);

        this->postlaunch();
        return *this;
    }

    Manager & launch(T * current, T * next, const int size) {
        return launch(current, next, size, size);
    }
};

} // update

template<class T>
using Update = update::Manager<T>;

} // CASE

#endif // UPDATEM

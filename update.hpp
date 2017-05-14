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
class Parallel : private map::Parallel<Job<T>> {
    enum class Access { Open, Closed };
    Access access = Access::Closed;

public:
    Parallel() {
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

    Parallel & launch(T * current, T * next, const int size, const int subset) {
        wait();
        assert(access == Access::Open);
        
        assert(current != nullptr);
        assert(next != nullptr);
        assert(current != next);
        assert(size >= 0);
        assert(subset >= 0);

        if (current < next)
            assert(current + size <= next);
        else
            assert(current >= next + size);

        // TODO check whether memmove is faster than current mechanism
        //std::memmove(next, current, size); 

        for (auto & job : this->jobs) {
            job.upload(current, next, size, subset);
            job.launch();
        }

        access = Access::Closed;
        return *this;
    }

    Parallel & launch(T * current, T * next, const int size) {
        return launch(current, next, size, size);
    }

    void terminate() {
        wait();
        for (auto & job : this->jobs) {
            job.terminate();
            job.upload(nullptr, nullptr, 0, 0);
            job.launch();
        }
        for (auto & job : this->jobs)
            job.thread.join();
        this->jobs.clear();
    }
};

} // update

template<class T>
using Update = update::Parallel<T>;

} // CASE

#endif // UPDATEM

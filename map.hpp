#ifndef CASE_MAP
#define CASE_MAP

#include <thread>
#include <mutex>
#include <list>
#include "timer.hpp"

namespace CASE {
namespace map {

template<class T>
class Parallel {
protected:
    std::list<T> jobs;

public:
    double job_duration() const {
        double total_duration = 0.0;
        for (const auto & job : jobs)
            total_duration += job.duration();
        return total_duration;
    }

    double actual_duration() const {
        return job_duration() / jobs.size();
    }

    void init() {
        const auto n_threads = std::thread::hardware_concurrency() - 1;
        for (std::size_t n = 0; n < n_threads; n++) {
            jobs.emplace_back(n, n_threads);
            auto & job = jobs.back();
            job.thread = std::thread{[&job]{ job.run(); }};
        }
    }

    void wait() {
        for (auto & job : jobs)
            job.wait();
    }
};

template<class T>
class Manager {
    enum class Access { Open, Closed };
    Access access = Access::Closed;

protected:
    T parallel;

    inline void prelaunch() const {
        assert(access == Access::Open);
    }

    inline void postlaunch() {
        access = Access::Closed;
    }

public:
    Manager() {
        parallel.init();
        parallel.wait();
        access = Access::Open;
    }

    ~Manager() {
        parallel.terminate();
    }

    Manager & wait() {
        if (access == Access::Open)
            return *this;

        parallel.wait();

        access = Access::Open;
        return *this;
    }
};

} // map
} // CASE

#endif // MAP

#ifndef CASE_MAP
#define CASE_MAP

#include <thread>
#include <mutex>
#include <list>
#include "timer.hpp"
#include "trigger.hpp"

namespace CASE {
namespace map {

class Base {
public:
    virtual double job_duration() const { return 0.0; }
    virtual void init() {}
    virtual void wait() {}
    virtual void terminate() {}
    virtual ~Base() {}
};

class Serial : public Base {
protected:
    CASE::Timer timer;

public:
    double job_duration() const override {
        return timer.dt();
    }
};

template<class T>
class Parallel : public Base {
protected:
    std::list<T> jobs;

public:
    double job_duration() const override {
        double total_duration = 0.0;
        for (const auto & job : jobs)
            total_duration += job.dt();
        return total_duration / jobs.size();
    }

    void init() override {
        const auto n_threads = std::thread::hardware_concurrency() - 1;
        for (std::size_t n = 0; n < n_threads; n++) {
            jobs.emplace_back(n, n_threads);
            auto & job = jobs.back();
            job.thread = std::thread{[&job]{ job.run(); }};
        }
    }

    void wait() override {
        for (auto & job : jobs)
            job.wait();
    }
};

} // map
} // CASE

#endif // MAP

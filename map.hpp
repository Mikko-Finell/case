#ifndef CASE_MAP
#define CASE_MAP

#include <thread>
#include <list>

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

    virtual void init() {
        const auto threads = std::thread::hardware_concurrency() - 1;
        for (std::size_t n = 0; n < threads; n++) {
            jobs.emplace_back(n, threads);
            auto & job = jobs.back();
            job.thread = std::thread{[&job]{ job.run(); }};
        }
    }

    void wait() {
        for (auto & job : jobs)
            job.wait();
    }
};

} // map
} // CASE

#endif // MAP

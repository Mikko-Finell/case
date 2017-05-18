#ifndef CASE_JOB
#define CASE_JOB

#include <mutex>
#include <thread>
#include <condition_variable>
#include "timer.hpp"

namespace CASE {
namespace job {

class Base {
    virtual void execute() = 0;

protected:
    const int nth;
    const int n_threads;
    Timer timer;

    std::condition_variable cv_done;
    std::condition_variable cv_launch;
    std::mutex              mutex_done;
    std::mutex              mutex_launch;
    bool flag_terminate     = false;
    bool flag_launch        = false;
    bool flag_done          = false;

public:
    std::thread thread;

    Base(const int n, const int thread_count)
        : nth(n), n_threads(thread_count)
    {}

    void wait() {
        std::unique_lock<std::mutex> lock_done{mutex_done};
        cv_done.wait(lock_done, [this]{ return flag_done; });
        flag_done = false;
    }

    void launch() {
        {
            std::lock_guard<std::mutex> lock_launch{mutex_launch};
            flag_launch = true;
        }
        cv_launch.notify_one();
    }

    void terminate() {
        flag_terminate = true;
    }

    double duration() const {
        return timer.dt();
    }

    void run() {
        while (true) {
            {
                std::unique_lock<std::mutex> lock_launch{mutex_launch};
                flag_launch = false;
                {
                    std::lock_guard<std::mutex> lock_done{mutex_done};
                    flag_done = true;
                }
                cv_done.notify_one();
                cv_launch.wait(lock_launch, [this]{ return flag_launch; });
            }
            if (flag_terminate)
                return;

            timer.start();
            execute();
            timer.reset();
        }
    }
};

} // job
} // CASE

#endif // CASE_JOB


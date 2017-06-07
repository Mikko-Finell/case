/* Author: Mikko Finell
 * License: Public Domain */

#ifndef CASE_JOB
#define CASE_JOB

#include <thread>
#include <mutex>
#include <condition_variable>

namespace CASE {

class Job {
    enum class Access { Open, Closed };
    Access access = Access::Closed;

    std::condition_variable cv_done;
    std::condition_variable cv_launch;
    std::mutex              mutex_done;
    std::mutex              mutex_launch;
    bool flag_terminate     = false;
    bool flag_launch        = false;
    bool flag_done          = false;

    virtual void execute() = 0;

protected:
    const int nth;
    const int n_threads;

public:
    std::thread thread;

    Job(const int n = 0, const int thread_count = 1)
        : nth(n), n_threads(thread_count)
    {}

    ~Job() {
        terminate();
    }

    void wait() {
        if (access == Access::Closed) {
            std::unique_lock<std::mutex> lock_done{mutex_done};
            cv_done.wait(lock_done, [this]{ return flag_done; });
            flag_done = false;
            access = Access::Open;
        }
    }

    void launch() {
        wait();
        {
            std::lock_guard<std::mutex> lock_launch{mutex_launch};
            flag_launch = true;
            access = Access::Closed;
        }
        cv_launch.notify_one();
    }

    void terminate() {
        if (thread.joinable()) {
            wait();
            flag_terminate = true;
            launch();
            thread.join();
        }
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
                break;
            else
                execute();
        }
        std::lock_guard<std::mutex> lock_done{mutex_done};
        flag_done = true;
        access = Access::Open;
    }
};

} // CASE

#endif // JOB

#ifndef CASE_LOG
#define CASE_LOG

#include <chrono>
#include <atomic>
#include <thread>
#include <mutex>
#include <sstream>
#include <fstream>

namespace CASE {

static constexpr char endl = '\n';

class Log {
    std::thread thread;
    std::mutex mutex;
    std::stringstream stream;
    std::atomic<bool> terminate{false};

public:
    Log(const std::string & filename) {
        thread = std::thread{[this, filename]
        {
            std::ofstream file{filename, std::ios::out};
            if (file.is_open() == false)
                throw std::runtime_error{"unable to open file" + filename};

            while (true) {
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
                mutex.lock();
                file << stream.rdbuf();
                mutex.unlock();
                if (terminate)
                    break;
            }
            file.close();
        }};
    }

    ~Log() {
        terminate = true;
        thread.join();
    }

    template<typename T>
    Log & out(const T & t) {
        std::lock_guard<std::mutex> lock{mutex};
        stream << t << '\n';
        return *this;
    }

    template<typename T>
    Log & operator<<(const T & t) {
        std::lock_guard<std::mutex> lock{mutex};
        stream << t;
        return *this;
    }
};

} // CASE

#endif // LOG

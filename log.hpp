/* Author: Mikko Finell
 * License: Public Domain */

#ifndef CASE_LOG
#define CASE_LOG

#include <ctime>
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
    std::atomic<bool> running{true};

public:
    Log(const std::string & filename) {
        thread = std::thread{[this, filename]
        {
            using namespace std::chrono;

            std::ofstream file{filename, std::ios::out};
            if (file.is_open() == false)
                throw std::runtime_error{"unable to open \"" + filename + "\""};

            const auto time = system_clock::to_time_t(system_clock::now());
            stream << "# " << std::ctime(&time);

            while (running) {
                std::this_thread::sleep_for(milliseconds(200));

                if (stream.fail())
                    throw std::runtime_error{"stream failbit"};

                if (file.fail())
                    throw std::runtime_error{"file failbit"};

                mutex.lock();
                const auto str = stream.str();
                stream.str("");
                mutex.unlock();

                file << str;
            }
            file.close();
        }};
    }

    ~Log() {
        running = false;
        thread.join();
    }

    template<typename T>
    Log & out(const T & t) {
        std::lock_guard<std::mutex> lock{mutex};
        stream << t << endl;
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

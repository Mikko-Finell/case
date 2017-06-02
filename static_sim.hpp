#ifndef CASE_STATIC_SIM
#define CASE_STATIC_SIM

#include <cassert>

#include <iostream>
#include <list>
#include <vector>
#include <algorithm>

#include <thread>
#include <mutex>
#include <thread>
#include <condition_variable>

#include <SFML/Graphics.hpp>

#include "random.hpp"
#include "array_buffer.hpp"
#include "graphics.hpp"
#include "timer.hpp"
#include "events.hpp"

namespace CASE {

template <class T>
class Job {
    Uniform random;
    std::vector<int> indices;
    T * current = nullptr;
    T * next = nullptr;
    volatile int array_size = 0;
    volatile int subset = 0;

    const int nth;
    const int n_threads;

    std::condition_variable cv_done;
    std::condition_variable cv_launch;
    std::mutex              mutex_done;
    std::mutex              mutex_launch;
    bool flag_terminate     = false;
    bool flag_launch        = false;
    bool flag_done          = false;

    void execute() {
        for (auto i = nth; i < array_size; i += n_threads) {
            indices.push_back(i);
            next[i] = current[i];
        }
        if (subset < array_size) {
            random.shuffle(indices);

            for (auto i = nth; i < subset; i += n_threads) {
                const auto index = indices.back();
                indices.pop_back();
                current[index].update(next[index]);
            }
        }
        else {
            for (auto i = nth; i < array_size; i += n_threads)
                current[i].update(next[i]);
        }
        indices.clear();
    }

public:
    std::thread thread;

    Job(const int n, const int thread_count)
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

    void upload(T * first, T * second, const int count, const int ss) {
        current = first;
        next = second;
        array_size = count;
        subset = ss;
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

            execute();
        }
    }
};


template<class T>
class Parallel {
protected:
    std::list<Job<T>> jobs;

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



template<class T>
class Update : private map::Parallel<T> {
    enum class Access { Open, Closed };
    Access access = Access::Closed;

public:
    Update() {
        map::Parallel<Job<T>>::init();
        wait();
    }

    ~Update() {
        terminate();
    }

    void wait() {
        if (access == Access::Open)
            return;

        map::Parallel<Job<T>>::wait();

        access = Access::Open;
    }

    Update & launch(T * current, T * next, const int size, const int subset) {
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

        for (auto & job : this->jobs) {
            job.upload(current, next, size, subset);
            job.launch();
        }

        access = Access::Closed;
        return *this;
    }

    Update & launch(T * current, T * next, const int size) {
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

template<class Config>
void Static() {
    Config config;
    using Agent = typename Config::Agent;
    assert(std::is_trivially_copyable<Agent>::value == true);

    const auto size      = config.columns * config.rows;
    const auto subset    = config.subset;
    auto agents          = new Agent[size * 2];
    auto world           = ArrayBuffer<Agent>{agents, agents + size};
    auto framerate       = config.framerate;

    sf::RenderWindow window;
    const auto win_w = config.columns * config.cell_size;
    const auto win_h = config.rows * config.cell_size;
    window.create(sf::VideoMode(win_w, win_h), config.title);
    window.setKeyRepeatEnabled(false);
    window.setVerticalSyncEnabled(true);

    Update<Agent> update;
    graphics::Parallel<Agent, 4> graphics{window};

    auto reset = [&update, &config, &world]() {
        update.wait();
        config.init(world.next());
    };
    
    bool pause = false;
    bool running = true;
    Timer timer;
    timer.start();

    config.init(world.next());
    graphics.draw(world.next(), size);
    graphics.display();

    while (running) {
        bool single_step = false;
        bool update_frame = false;
        eventhandling(window, running, pause, single_step, framerate, reset);

        if (pause) {
            if (single_step)
                update_frame = true;
        }
        else if (timer.dt() >= 1000.0 / framerate) {
            timer.reset();
            update_frame = true;
        }
        if (update_frame) {
            world.flip();
            update.wait();
            config.preprocessing();
            update.launch(world.current(), world.next(), size, subset);
        }
        graphics.draw(world.current(), size);
        graphics.clear(config.bgcolor);
        graphics.display();
    }

    update.terminate();
    graphics.terminate();
    delete [] agents;
}

} // CASE

#endif

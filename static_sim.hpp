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
#include "timer.hpp"
#include "events.hpp"

namespace CASE {

class Job {
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

template <class T>
class GraphicsJob : public Job {

    const T * objects = nullptr;
    sf::Vertex * vertices = nullptr;
    volatile int array_size = 0;

    void execute() override {
        constexpr int VS_PER_OBJ = 4;
        for (auto i = nth; i < array_size; i += n_threads)
            objects[i].draw(vertices + i * VS_PER_OBJ);
    }

public:
    using Job::Job;

    void upload(const T * objs, sf::Vertex * vs, const int count) {
        objects = objs;
        vertices = vs;
        array_size = count;
    }
};

template <class T>
class UpdateJob : public Job {
    Uniform random;
    std::vector<int> indices;
    T * current = nullptr;
    T * next = nullptr;
    volatile int array_size = 0;
    volatile int subset = 0;

    void execute() override {
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
    using Job::Job;

    void upload(T * first, T * second, const int count, const int ss) {
        current = first;
        next = second;
        array_size = count;
        subset = ss;
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

    // set up SFML
    sf::RenderWindow window;
    const auto win_w = config.columns * config.cell_size;
    const auto win_h = config.rows * config.cell_size;
    window.create(sf::VideoMode(win_w, win_h), config.title);
    window.setKeyRepeatEnabled(false);
    window.setVerticalSyncEnabled(true);

    enum Access { Open, Closed };

    auto wait = [](auto && task) {
       if (task.access != Open) {
            for (auto & job : task.jobs)
                job.wait();
            task.access = Open;
        }
    };

    // initialize update threads
    struct Update {
        std::list<UpdateJob<Agent>> jobs;
        Access access = Access::Closed;
    } update;

    // set number of threads used by update and graphics, with a minimum of one
    constexpr unsigned int mt = 1;
    const int threads = std::max(std::thread::hardware_concurrency() - 1, mt);
    for (auto i = 0; i < threads; i++) {
        update.jobs.emplace_back(i, threads);
        auto & job = update.jobs.back();
        job.thread = std::thread{[&job]{ job.run(); }};
    }

    wait(update);

    struct Graphics {
        std::list<GraphicsJob<Agent>> jobs;
        Access access = Access::Closed;
        std::vector<sf::Vertex> vertices[2];
        ArrayBuffer<std::vector<sf::Vertex>> vs{&vertices[0], &vertices[1]};
    } graphics;

    // initialize graphics threads
    for (auto i = 0; i < threads; i++) {
        graphics.jobs.emplace_back(i, threads);
        auto & job = graphics.jobs.back();
        job.thread = std::thread{[&job]{ job.run(); }};
    }

    wait(graphics);

    auto reset = [&]() {
        wait(update);
        wait(graphics);
        config.init(world.next());
    };

    auto fast_forward = [&](const auto factor) {
        const auto frames = std::pow(10, factor);
        std::cout << "Forwarding " << frames << " frames" << std::endl;
        for (auto i = 0; i < frames; i++) {
            wait(update);
            world.flip();
            for (auto & job : update.jobs) {
                job.upload(world.current(), world.next(), size, subset);
                job.launch();
            }
            update.access = Closed;
        }
    };
    
    bool pause = false;
    bool running = true;
    Timer timer;
    timer.start();

    config.init(world.next());

    while (running) {
        bool single_step = false;
        bool update_frame = false;
        eventhandling(window, running, pause, single_step, framerate,
                      reset, fast_forward);

        if (pause) {
            if (single_step)
                update_frame = true;
        }
        else if (timer.dt() >= 1000.0 / framerate) {
            timer.reset();
            update_frame = true;
        }

        if (update_frame) {
            wait(update);

            config.preprocessing();

            world.flip();
            for (auto & job : update.jobs) {
                job.upload(world.current(), world.next(), size, subset);
                job.launch();
            }
            update.access = Closed;
        }

        // draw
        wait(graphics);
        graphics.vs.flip(); // swap buffers
        graphics.vs.next()->resize(size * 4);
        for (auto & job : graphics.jobs) {
            job.upload(world.current(), graphics.vs.next()->data(), size);
            job.launch();
        }
        graphics.access = Closed;

        // display
        window.clear(config.bgcolor);
        auto & current = *graphics.vs.current();
        window.draw(current.data(), current.size(), sf::Quads);
        window.display();
    }

    // terminate update threads
    wait(update);
    for (auto & job : update.jobs) {
        job.terminate();
        job.upload(nullptr, nullptr, 0, 0);
        job.launch();
        job.thread.join();
    }
    update.jobs.clear();

    // graphics terminate
    wait(graphics);
    for (auto & job : graphics.jobs) {
        job.terminate();
        job.upload(nullptr, nullptr, 0);
        job.launch();
        job.thread.join();
    }
    graphics.jobs.clear();

    delete [] agents;
}

} // CASE

#endif

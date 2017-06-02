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

template <class T>
class GraphicsJob {
    const int nth;
    const int n_threads;

    std::condition_variable cv_done;
    std::condition_variable cv_launch;
    std::mutex              mutex_done;
    std::mutex              mutex_launch;
    bool flag_terminate     = false;
    bool flag_launch        = false;
    bool flag_done          = false;

    const T * objects = nullptr;
    sf::Vertex * vertices = nullptr;
    volatile int array_size = 0;

public:
    std::thread thread;

    GraphicsJob(const int n, const int thread_count)
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

            constexpr int VS_PER_OBJ = 4;
            for (auto i = nth; i < array_size; i += n_threads)
                objects[i].draw(vertices + i * VS_PER_OBJ);
        }
    }

    void upload(const T * objs, sf::Vertex * vs, const int count) {
        objects = objs;
        vertices = vs;
        array_size = count;
    }
};




template <class T>
class Graphics {

    enum class Access { Open, Closed };
    Access access = Access::Closed;
    std::list<GraphicsJob<T>> jobs;

    sf::RenderWindow * window = nullptr;
    std::vector<sf::Vertex> vertices[2];
    ArrayBuffer<std::vector<sf::Vertex>> vs{&vertices[0], &vertices[1]};

public:
    Graphics(sf::RenderWindow & w) : window(&w)
    {
        const auto threads = std::thread::hardware_concurrency() - 1;
        for (std::size_t n = 0; n < threads; n++) {
            jobs.emplace_back(n, threads);
            auto & job = jobs.back();
            job.thread = std::thread{[&job]{ job.run(); }};
        }
        wait();
    }

    ~Graphics() {
        terminate();
    }

    void wait() {
        if (access == Access::Open)
            return;

        for (auto & job : jobs)
            job.wait();

        access = Access::Open;
    }

    Graphics & draw(const T * objects, const int size) {
        assert(window != nullptr);
        assert(objects != nullptr);
        assert(size >= 0);

        wait();

        vs.flip(); // swap buffers
        vs.next()->resize(size * 4);

        for (auto & job : this->jobs) {
            job.upload(objects, vs.next()->data(), size);
            job.launch();
        }

        return *this;
    }

    void clear(const sf::Color color = sf::Color::White) {
        window->clear(color);
    }

    void display() {
        auto & current = *vs.current();
        window->draw(current.data(), current.size(), sf::Quads);
        window->display();
    }

    void terminate() {
        for (auto & job : this->jobs) {
            job.terminate();
            job.upload(nullptr, nullptr, 0);
            job.launch();
            job.thread.join();
        }
        this->jobs.clear();
    }
};





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

            // execute
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

    // Update variables
    enum Access { Open, Closed };
    std::list<Job<Agent>> jobs;

    // initialize update threads
    const auto threads = std::thread::hardware_concurrency() - 1;
    for (std::size_t n = 0; n < threads; n++) {
        jobs.emplace_back(n, threads);
        auto & job = jobs.back();
        job.thread = std::thread{[&job]{ job.run(); }};
    }
    for (auto & job : jobs)
        job.wait();
    Access access = Open;

    auto wait = [&access, &jobs]() {
       if (access != Open) {
            for (auto & job : jobs)
                job.wait();
            access = Open;
        }
    };

    auto reset = [&]() {
        wait();
        config.init(world.next());
    };

    Graphics<Agent> graphics{window};
    
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

            // Wait for threads to get ready
            wait();
            config.preprocessing();

            for (auto & job : jobs) {
                job.upload(world.current(), world.next(), size, subset);
                job.launch();
            }
            access = Closed;
        }
        graphics.draw(world.current(), size);
        graphics.clear(config.bgcolor);
        graphics.display();
    }

    // Terminate jobs
    wait();
    for (auto & job : jobs) {
        job.terminate();
        job.upload(nullptr, nullptr, 0, 0);
        job.launch();
    }
    for (auto & job : jobs)
        job.thread.join();
    jobs.clear();

    graphics.terminate();
    delete [] agents;
}

} // CASE

#endif

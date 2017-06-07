/* Author: Mikko Finell
 * License: Public Domain */

#ifndef CASE_STATIC_SIM
#define CASE_STATIC_SIM

#include <cassert>

#include <iostream>
#include <list>
#include <vector>
#include <algorithm>

#include <SFML/Graphics.hpp>

#include "job.hpp"
#include "random.hpp"
#include "pair.hpp"
#include "timer.hpp"
#include "events.hpp"

namespace CASE {

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
        wait();

        objects = objs;
        vertices = vs;
        array_size = count;
    }
};

template <class T>
class UpdateJob : public Job {
    Uniform<> random;
    std::vector<int> indices;
    T * current = nullptr;
    T * next = nullptr;
    volatile int array_size = 0;
    volatile int subset = 0;

    void execute() override {
        indices.clear();
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
    }

public:
    using Job::Job;

    void upload(T * first, T * second, const int count, const int ss) {
        wait();

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
    auto world           = Pair<Agent *>{agents, agents + size};
    auto framerate       = config.framerate;

    CAdjacent<Agent>::columns = config.columns;
    CAdjacent<Agent>::rows = config.rows;

    // set up SFML
    sf::RenderWindow window;
    const auto win_w = config.columns * config.cell_size;
    const auto win_h = config.rows * config.cell_size;
    window.create(sf::VideoMode(win_w, win_h), config.title);
    window.setKeyRepeatEnabled(false);
    window.setVerticalSyncEnabled(true);

    // set number of threads used by update and graphics, with a minimum of 1
    const int threads = std::max<int>(std::thread::hardware_concurrency()-1, 1);

    // initialize update threads
    std::list<UpdateJob<Agent>> update_jobs;
    for (auto i = 0; i < threads; i++) {
        update_jobs.emplace_back(i, threads);
        auto & job = update_jobs.back();
        job.thread = std::thread{[&job]{ job.run(); }};
    }

    // initialize graphics threads
    std::list<GraphicsJob<Agent>> graphics_jobs;
    Pair<std::vector<sf::Vertex>> vertices;
    for (auto i = 0; i < threads; i++) {
        graphics_jobs.emplace_back(i, threads);
        auto & job = graphics_jobs.back();
        job.thread = std::thread{[&job]{ job.run(); }};
    }

    auto reset = [&]() {
        for (auto & job : update_jobs) job.wait();
        for (auto & job : graphics_jobs) job.wait();
        config.init(world.next());
    };

    auto fast_forward = [&](const auto factor) {
        auto frames = std::pow(10, factor);
        std::cout << "Forwarding " << frames << " frames" << std::endl;
        while (frames--) {
            world.flip();
            for (auto & job : update_jobs) {
                job.upload(world.current(), world.next(), size, subset);
                job.launch();
            }
        }
    };
    
    bool pause = false;
    bool running = true;
    Timer timer;
    timer.start();

    config.init(world.next());
    
    while (running) {
        bool single_step = false;
        bool update = false;
        eventhandling(window, running, pause, single_step, framerate,
                      reset, fast_forward);

        if (pause) {
            if (single_step)
                update = true;
        }
        else if (timer.dt() >= 1000.0 / framerate) {
            timer.reset();
            update = true;
        }

        if (update) {
            for (auto & job : update_jobs)
                job.wait();

            config.postprocessing(world.current());

            world.flip();
            for (auto & job : update_jobs) {
                job.upload(world.current(), world.next(), size, subset);
                job.launch();
            }
        }

        // draw
        vertices.flip();
        vertices.next().resize(size * 4);
        for (auto & job : graphics_jobs) {
            job.upload(world.current(), vertices.next().data(), size);
            job.launch();
        }

        // display
        window.clear(config.bgcolor);
        auto & current = vertices.current();
        window.draw(current.data(), current.size(), sf::Quads);
        window.display();
    }

    // terminate update threads
    for (auto & job : update_jobs)
        job.terminate();

    for (auto & job : graphics_jobs)
        job.terminate();

    delete [] agents;
}

} // CASE

#endif

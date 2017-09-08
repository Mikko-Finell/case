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
#include "log.hpp"

namespace CASE {

template <class T>
class UpdateJob : public Job {
    Uniform<> random;
    T * current = nullptr;
    T * next = nullptr;
    int array_size = 0;

    void execute() override {
        for (auto i = nth; i < array_size; i += n_threads) {
            next[i] = current[i];
            current[i].update(next[i]);
        }
    }

public:
    using Job::Job;

    void upload(T * first, T * second, const int count) {
        wait();
        current = first;
        next = second;
        array_size = count;
    }
};

template<class Config>
void Static() {
    Config config;
    using Agent = typename Config::Agent;
    assert(std::is_trivially_copyable<Agent>::value == true);

    const auto size      = config.columns * config.rows;
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

    auto update = [&]() {
        for (auto & job : update_jobs)
            job.wait();
        config.postprocessing(world.current());
        world.flip();
        for (auto & job : update_jobs) {
            job.upload(world.current(), world.next(), size);
            job.launch();
        }
    };

    std::vector<sf::Vertex> vertices;
    vertices.resize(size * 4);

    auto reset = [&]() {
        for (auto & job : update_jobs) job.wait();
        config.init(world.next());
    };

    auto fast_forward = [&](const auto factor) {
        auto frames = std::pow(10, factor);
        std::cout << "Forwarding " << frames << " frames" << std::endl;
        static Timer timer; timer.start();
        while (frames--)
            update();
        std::cout << timer.reset() << "ms\n";
    };
    
    bool pause = false;
    bool running = true;
    double dt = 0.0;
    Timer timer;

    reset();

    while (running) {
        bool step = false;

        eventhandling(window, running, pause, step, framerate,
                      reset, fast_forward);
        if (pause) {
            if (step)
                update();

            timer.reset();
            dt = 0.0;
        }
        else {
            const auto frame_time = 1000.0 / framerate;
            dt += timer.reset();
            if (dt > frame_time) {
                dt -= frame_time;

                update();
            }
        }

        // render
        auto & current_agents = world.current();
        for (auto i = 0; i < size; i++)
            current_agents[i].draw(&vertices[0] + i * 4);
        
        // display
        window.clear(config.bgcolor);
        window.draw(&vertices[0], vertices.size(), sf::Quads);
        window.display();
    }

    // terminate update threads
    for (auto & job : update_jobs)
        job.terminate();

    delete [] agents;
}

} // CASE

#endif

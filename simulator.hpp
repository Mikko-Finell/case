#ifndef CASE_SIM
#define CASE_SIM

#include <iostream>

#include "array_buffer.hpp"
#include "dynamic_update.hpp"
#include "static_update.hpp"
#include "graphics.hpp"
#include "grid.hpp"
#include "gc.hpp"
#include "timer.hpp"

namespace CASE {

namespace simulator {

void eventhandling(sf::RenderWindow & window, bool & running, bool & pause,
                   bool & single_step, double & framerate,
                   const std::function<void(bool &, bool &)> & reset//,
                   /*std::function<void()> fast_forward*/);

template<class Config>
void Dynamic() {
    Config config;
    using Agent = typename Config::Agent;
    using Cell = typename Config::Cell;

    constexpr auto array_size = config.max_agents;
    constexpr auto subset     = config.subset;

    auto agents     = new Agent[array_size];
    auto framerate  = config.framerate;

    sf::RenderWindow window;
    const auto win_w = config.columns * config.cell_size;
    const auto win_h = config.rows * config.cell_size;
    window.create(sf::VideoMode(win_w, win_h), config.title);
    window.setKeyRepeatEnabled(false);
    window.setVerticalSyncEnabled(true);

    graphics::Dynamic<Agent> graphics{window};
    CASE::Grid<Cell> grid{config.columns, config.rows};

    config.init(agents, grid);
    GarbageCollector<Agent> gc{agents, array_size};
    gc.compact();

    graphics.draw(agents, gc.count());
    graphics.clear(config.bgcolor);
    graphics.display();

    update::Dynamic<Agent> update{agents};

    auto reset = [&config, &agents, &grid, &gc]
                 (bool & pause, bool & single_step)
    {
        config.init(agents, grid);
        if (pause)
            single_step = true;
    };

    int size = 0;
    bool pause = false;
    bool running = true;
    Timer timer;
    timer.start();

    while (running) {
        bool single_step = false;
        eventhandling(window, running, pause, single_step, framerate, reset);
        if (pause) {
            if (single_step) {
                size = gc.compact();
                update.launch(size, subset);
            }
        }
        else if (timer.dt() >= 1000.0 / framerate) {
            timer.reset();
            size = gc.compact();
            update.launch(size, subset);
        }
        graphics.draw(agents, size);
        graphics.clear(config.bgcolor);
        graphics.display();
    }

    delete [] agents;
}

template<class Config>
void Static() {
    Config config;
    using Agent = typename Config::Agent;

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

    update::Static<Agent> update;
    graphics::Parallel<Agent, 4> graphics{window};

    /*auto fast_forward=[size, subset](auto & update, auto & world, const int n)
    {
        auto generations = std::pow(10, n);
        if (n > 1)
            generations -= std::pow(10, n - 1);

        for (auto i = 0; i < generations; i++) {
            update.launch(world.current(), world.next(), size, subset);
            world.flip();
        }
    };*/
    auto reset = [&update, &config, &world](bool & pause, bool & single_step) {
        update.wait();
        config.init(world.next());
        if (pause)
            single_step = true;
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
        eventhandling(window, running, pause, single_step, framerate,
                      reset);//, fast_forward);

        if (pause) {
            if (single_step) {
                world.flip();
                update.launch(world.current(), world.next(), size, subset);
            }
        }
        else if (timer.dt() >= 1000.0 / framerate) {
            timer.reset();

            world.flip();
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

void eventhandling(sf::RenderWindow & window, bool & running, bool & pause,
                   bool & single_step, double & framerate,
                   const std::function<void(bool &, bool &)> & reset//,
                   /*std::function<void()> fast_forward*/)
{
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::KeyPressed) {
            switch (event.key.code) {
                case sf::Keyboard::Pause:
                    pause = !pause;
                    continue;

                case sf::Keyboard::R:
                    using _ = sf::Keyboard;
                    if (_::isKeyPressed(_::RControl)
                    || _::isKeyPressed(_::LControl))
                case sf::Keyboard::F5:
                    {
                        reset(pause, single_step);
                    }
                    continue;

                case sf::Keyboard::Q:
                case sf::Keyboard::End:
                case sf::Keyboard::Escape:
                    running = false;
                    continue;

                case sf::Keyboard::Right:
                    if (pause)
                        single_step = true;
                    else
                        pause = true;
                    continue;

                case sf::Keyboard::Left:
                    continue;

                case sf::Keyboard::Up:
                    if (pause)
                        pause = false;
                    else
                        framerate += 5.0;
                    continue;

                case sf::Keyboard::Down:
                    if (pause)
                        pause = false;
                    else
                        framerate -= 5.0;
                    continue;
                /*
                case sf::Keyboard::Num0:
                    fast_forward(10);
                case sf::Keyboard::Num9:
                    fast_forward(9);
                case sf::Keyboard::Num8:
                    fast_forward(8);
                case sf::Keyboard::Num7:
                    fast_forward(7);
                case sf::Keyboard::Num6:
                    fast_forward(6);
                case sf::Keyboard::Num5:
                    fast_forward(5);
                case sf::Keyboard::Num4:
                    fast_forward(4);
                case sf::Keyboard::Num3:
                    fast_forward(3);
                case sf::Keyboard::Num2:
                    fast_forward(2);
                case sf::Keyboard::Num1:
                    fast_forward(1);
                    continue;
                */
                default:
                    continue;
            }
        }
        if (event.type == sf::Event::Closed) {
            running = false;
            continue;
        }
    }
}

} // simulator

} // CASE

#endif // CASE_SIM

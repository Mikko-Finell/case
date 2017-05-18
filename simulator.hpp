#ifndef CASE_SIM
#define CASE_SIM

#include <iostream>

#include <SFML/Graphics.hpp>
#include <algorithm>

#include "array_buffer.hpp"
#include "dynamic_update.hpp"
#include "static_update.hpp"
#include "graphics.hpp"
#include "grid.hpp"
#include "world.hpp"
#include "gc.hpp"
#include "timer.hpp"
#include "log.hpp"

namespace CASE {
namespace simulator {

void eventhandling(sf::RenderWindow & window, bool & running, bool & pause,
                   bool & single_step, double & framerate,
                   const std::function<void(bool &, bool &)> & reset);

template<class Config>
void Dynamic() {
    Config config;
    using Agent = typename Config::Agent;
    using Cell = typename Config::Cell;

    constexpr auto array_size = config.max_agents;
    constexpr auto subset     = config.subset;
    auto framerate            = config.framerate;

    sf::RenderWindow window;
    const auto win_w = config.columns * config.cell_size;
    const auto win_h = config.rows * config.cell_size;
    window.create(sf::VideoMode(win_w, win_h), config.title);
    window.setKeyRepeatEnabled(false);
    window.setVerticalSyncEnabled(true);

    CASE::Grid<Cell> grid{config.columns, config.rows};
    World<Cell> world{array_size, grid};
    graphics::dynamic::DoubleBuffer<Agent> graphics{window};
    update::Dynamic<Agent> update{world.agents};

    config.init(world);

    graphics.clear(config.bgcolor);
    graphics.draw(world.agents, world.count());

    auto reset = [&config, &world](bool & pause, bool & single_step)
    {
        config.init(world);
        if (pause)
            single_step = true;
    };

    bool pause = false;
    bool running = true;
    Timer timer;
    timer.start();

    Log perf{"update.dat"};
    Log pop{"pop.dat"};

    while (running) {
        bool single_step = false;
        eventhandling(window, running, pause, single_step, framerate, reset);
        if (pause) {
            if (single_step)
                update.launch(world, subset);
        }
        else if (timer.dt() >= 1000.0 / framerate) {
            timer.reset();

            Timer updt;
            updt.start();
            update.launch(world, subset);
            //perf.out(updt.dt());
            //pop.out(world.count());
        }
        graphics.clear(config.bgcolor);
        graphics.draw(world.agents, world.count());
        graphics.display();
    }
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
        eventhandling(window, running, pause, single_step, framerate, reset);

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
                   const std::function<void(bool &, bool &)> & reset)
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

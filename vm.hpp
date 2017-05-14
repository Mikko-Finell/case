#ifndef CASE_VM
#define CASE_VM

#include <iostream>

#include "array_buffer.hpp"
#include "update.hpp"
#include "graphics.hpp"
#include "timer.hpp"

namespace CASE {

namespace vm {

template<class Simulation>
void Totalistic() {
    Simulation sim;
    using Agent = typename Simulation::Agent;

    const auto size      = sim.columns * sim.rows;
    const auto subset    = sim.subset;
    auto agents          = new Agent[size * 2];
    auto world           = ArrayBuffer<Agent>{agents, agents + size};
    auto framerate       = sim.framerate;

    sf::RenderWindow window;
    const auto win_w = sim.columns * sim.cell_size;
    const auto win_h = sim.rows * sim.cell_size;
    window.create(sf::VideoMode(win_w, win_h), sim.title);
    window.setKeyRepeatEnabled(false);
    window.setVerticalSyncEnabled(true);

    typename Simulation::Graphics graphics{window};
    typename Simulation::Update update{};

    auto fast_forward = [size, subset](auto & update, auto & world, const int n)
    {
        auto generations = std::pow(10, n);
        if (n > 1)
            generations -= std::pow(10, n - 1);

        update.wait();
        for (auto i = 0; i < generations; i++) {
            update.launch(world.current(), world.next(), size, subset);
            update.wait();
            world.flip();
        }
    };

    bool pause = false;
    bool running = true;
    Timer timer;
    timer.start();

    sim.init(world.next());
    graphics.clear(sim.bgcolor);

    while (running) {
        sf::Event event;
        bool single_step = false;
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
                        {
                    case sf::Keyboard::F5:
                            update.wait();
                            sim.init(world.next());
                            if (pause)
                                single_step = true;
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

                    case sf::Keyboard::Num0:
                        fast_forward(update, world, 10);
                    case sf::Keyboard::Num9:
                        fast_forward(update, world, 9);
                    case sf::Keyboard::Num8:
                        fast_forward(update, world, 8);
                    case sf::Keyboard::Num7:
                        fast_forward(update, world, 7);
                    case sf::Keyboard::Num6:
                        fast_forward(update, world, 6);
                    case sf::Keyboard::Num5:
                        fast_forward(update, world, 5);
                    case sf::Keyboard::Num4:
                        fast_forward(update, world, 4);
                    case sf::Keyboard::Num3:
                        fast_forward(update, world, 3);
                    case sf::Keyboard::Num2:
                        fast_forward(update, world, 2);
                    case sf::Keyboard::Num1:
                        fast_forward(update, world, 1);
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
        graphics.clear(sim.bgcolor);
        graphics.display();
    }

    update.terminate();
    graphics.terminate();
    delete [] agents;
}

} // vm

} // CASE

#endif // CASE_VM

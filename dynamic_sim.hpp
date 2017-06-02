#ifndef CASE_SIM
#define CASE_SIM

#include <iostream>

#include <SFML/Graphics.hpp>

#include "array_buffer.hpp"
#include "graphics.hpp"
#include "grid.hpp"
#include "agent_manager.hpp"
#include "timer.hpp"
#include "log.hpp"
#include "events.hpp"

namespace CASE {

template<class Config>
void Dynamic() {
    Config config;

    using Agent = typename Config::Agent;
    assert(std::is_trivially_copyable<Agent>::value == true);
    using Cell = typename Config::Cell;

    auto framerate = config.framerate;

    sf::RenderWindow window;
    const auto win_w = config.columns * config.cell_size;
    const auto win_h = config.rows * config.cell_size;
    window.create(sf::VideoMode(win_w, win_h), config.title);
    window.setKeyRepeatEnabled(false);
    window.setVerticalSyncEnabled(true);

    Neighbors<Cell>::columns = config.columns;
    Neighbors<Cell>::rows = config.rows;

    // note Grid must be constructed before graphics
    AgentManager<Agent> manager{config.columns * config.rows * Cell::depth};
    Grid<Cell> grid{config.columns, config.rows, manager};
    graphics::dynamic::SingleBuffer<Cell> graphics{window};

    auto reset = [&config, &grid, &manager]()
    {
        config.init(grid, manager);
    };
    reset();

    bool pause = false;
    bool running = true;
    Timer timer;
    timer.start();

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
            manager.update();
            config.postprocessing(grid);
        }

        graphics.clear(config.bgcolor);
        graphics.draw(grid.cells, grid.cell_count());
        graphics.display();
    }
}

} // CASE

#endif // CASE_SIM

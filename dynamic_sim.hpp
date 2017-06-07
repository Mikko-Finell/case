/* Author: Mikko Finell
 * License: Public Domain */

#ifndef CASE_SIM
#define CASE_SIM

#include <SFML/Graphics.hpp>

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

    AgentManager<Agent> manager{config.columns * config.rows * Cell::depth};
    Grid<Cell> grid{config.columns, config.rows, manager};
    std::vector<sf::Vertex> vertices;

    auto reset = [&config, &grid, &manager]()
    {
        config.init(grid, manager);
    };
    reset();

    auto fast_forward = [&](const auto factor) {
        auto frames = std::pow(10, factor);
        std::cout << "Forwarding " << frames << " frames" << std::endl;
        while (frames--)
            manager.update();
    };

    bool pause = false;
    bool running = true;
    Timer timer;
    timer.start();

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
            manager.update();
            config.postprocessing(grid);
        }

        window.clear(config.bgcolor);

        vertices.clear();
        for (auto i = 0; i < grid.cell_count(); i++)
            grid.cells[i].draw(vertices);

        window.draw(vertices.data(), vertices.size(), sf::Quads);
        window.display();
    }
}

} // CASE

#endif // CASE_SIM

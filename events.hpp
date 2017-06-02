#ifndef CASE_EVENTS
#define CASE_EVENTS

#include <SFML/Graphics.hpp>

namespace CASE {

inline 
void eventhandling(sf::RenderWindow & window, bool & running, bool & pause,
                   bool & single_step, double & framerate,
                   const std::function<void()> & reset)
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
                        reset();
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

} // CASE

#endif // EVENTS

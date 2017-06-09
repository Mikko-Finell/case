/* Author: Mikko Finell
 * License: Public Domain */

#ifndef CASE_EVENTS
#define CASE_EVENTS

#include <SFML/Graphics.hpp>

namespace CASE {

inline 
void eventhandling(sf::RenderWindow & window, bool & running, bool & pause,
                   bool & single_step, double & framerate,
                   const std::function<void()> & reset,
                   const std::function<void(const int)> & fast_forward)
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
                        framerate = std::max<double>(1, framerate - 5);
                    continue;

                case sf::Keyboard::Num1:
                    fast_forward(1);
                    continue;

                case sf::Keyboard::Num2:
                    fast_forward(2);
                    continue;

                case sf::Keyboard::Num3:
                    fast_forward(3);
                    continue;

                case sf::Keyboard::Num4:
                    fast_forward(4);
                    continue;

                case sf::Keyboard::Num5:
                    fast_forward(5);
                    continue;

                case sf::Keyboard::Num6:
                    fast_forward(6);
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

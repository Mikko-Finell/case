#ifndef CASE_QUAD
#define CASE_QUAD

#include <SFML/Graphics.hpp>

namespace CASE {

namespace impl {
inline void setpos(sf::Vertex & vs, const int x, const int y) {
    vs.position.x = x;
    vs.position.y = y;
}

inline void setcolor(sf::Vertex & vs, const int r, const int g, const int b) {
    vs.color.r = r;
    vs.color.g = g;
    vs.color.b = b;
}
} // impl

inline
void quad(const int x, const int y, const int w, const int h,
          const int r, const int g, const int b, sf::Vertex * vs)
{
    using namespace impl;
    setpos(vs[0], x, y);
    setpos(vs[1], x + w, y);
    setpos(vs[2], x + w, y + h);
    setpos(vs[3], x, y + h);
    setcolor(vs[0], r, g, b);
    setcolor(vs[1], r, g, b);
    setcolor(vs[2], r, g, b);
    setcolor(vs[3], r, g, b);
}

inline
void quad(const int x, const int y, const int w, const int h, sf::Vertex * vs) {
    using namespace impl;
    setpos(vs[0], x, y);
    setpos(vs[1], x + w, y);
    setpos(vs[2], x + w, y + h);
    setpos(vs[3], x, y + h);
}

inline
void quad(const int r, const int g, const int b, sf::Vertex * vs) {
    using namespace impl;
    setcolor(vs[0], r, g, b);
    setcolor(vs[1], r, g, b);
    setcolor(vs[2], r, g, b);
    setcolor(vs[3], r, g, b);
}

template<class Position, class Size, class Color>
void quad(const Position & pos, const Size & size, const Color & color,
          sf::Vertex * vs)
{
    vs[0] = sf::Vertex{pos, color};
    vs[1] = sf::Vertex{pos + sf::Vector2f{size.x,0}, color};
    vs[2] = sf::Vertex{pos + size, color};
    vs[3] = sf::Vertex{pos + sf::Vector2f{0, size.y}, color};
}

template<class Position, class Size>
void quad(const Position & pos, const Size & size, sf::Vertex * vs) {
    vs[0].position = pos;
    vs[1].position = pos + sf::Vector2f{size.x,0};
    vs[2].position = pos + size;
    vs[3].position = pos + sf::Vector2f{0, size.y};
}

template<class Color>
void quad(const Color & color, sf::Vertex * vs) {
    vs[0].color = color;
    vs[1].color = color;
    vs[2].color = color;
    vs[3].color = color;
}

} // CASE

#endif // QUAD

#ifndef CASE_INDEX
#define CASE_INDEX

#include <cassert>

namespace CASE {

inline int wrap(const int n, const int size) {
    assert(size > 0);
    if (n >= 0)
        return n % size;
    else
        return ((n % size) + size) % size;
}

inline int index(const int x, const int y, const int size) {
    assert(size > 0);
    return y * size + x;
}

template<class T>
inline T & bda(T * t, int i, int c, int r, int x, int y) {
    return *(t+(((i%c+x)%c)+c)%c+((((i/c+y)%r)+r)%r)*c-i);
}

} // CASE

#endif // INDEX

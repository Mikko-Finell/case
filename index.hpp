/* Author: Mikko Finell
 * License: Public Domain */

#ifndef CASE_INDEX
#define CASE_INDEX

#include <cassert>

namespace CASE {

// n mod size with floored division, so as to wrap backwards around negative n
inline int wrap(const int n, const int size) {
    assert(size > 0);
    return ((n % size) + size) % size;
}

// row-major major matrix index 
inline int index(const int x, const int y, const int size) {
    assert(size > 0);
    return y * size + x;
}

namespace _impl {
template<class T>
inline T & bda(T * t, int i, int c, int r, int x, int y) {
    return *(t+(((i%c+x)%c)+c)%c+((((i/c+y)%r)+r)%r)*c-i);
}
} // __impl

} // CASE

#endif // INDEX

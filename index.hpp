/* Author: Mikko Finell
 * License: Public Domain */

#ifndef CASE_INDEX
#define CASE_INDEX

#include <cassert>
#include "helper.hpp"

namespace CASE {

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
} // _impl

} // CASE

#endif // INDEX

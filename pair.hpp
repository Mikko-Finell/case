/* Author: Mikko Finell
 * License: Public Domain */

#ifndef CASE_PAIR
#define CASE_PAIR

namespace CASE {

template<class T>
class Pair {
    T a, b;
    bool flipbit = true;

public:
    Pair() {}

    Pair(T first, T second) : a(first), b(second)
    {}

    inline T & current() {
        return flipbit ? a : b;
    }

    void current(const T & t) {
        flipbit ? a = t : b = t;
    }

    inline T & next() {
        return flipbit ? b : a;
    }

    void next(const T & t) {
        flipbit ? b = t : a = t;
    }

    inline T & flip() {
        flipbit = !flipbit;
        return current();
    }
};

} // CASE

#endif // PAIR

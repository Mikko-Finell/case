#ifndef CASE_NEIGHBORS
#define CASE_NEIGHBORS

#include <stdexcept>
#include <cassert>

namespace CASE {

template<class T>
class Neighbors {
    T * array[9] = {
        nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr
    };
    inline int index(const int x, const int y) const {
        switch (y) {
            case -1:
                switch (x) {
                    case -1: return 0;
                    case  0: return 1;
                    case  1: return 2;
                }
            case 0:
                switch (x) {
                    case -1: return 3;
                    case  0: return 4;
                    case  1: return 5;
                }
            case 1:
                switch(x) {
                    case -1: return 6;
                    case  0: return 7;
                    case  1: return 8;
                }
            default: throw std::invalid_argument{"invalid index"};
        }
    }
public:
    T & operator()(const int x, const int y) {
        return *array[index(x, y)];
    }

    const T & operator()(const int x, const int y) const {
        return *array[index(x, y)];
    }

    void set(const int x, const int y, T & t) {
        array[index(x, y)] = &t;
    }

    T * ptr(const int x, const int y) {
        return array[index(x, y)];
    }
};

} // CASE

#endif // NEIGHBORS

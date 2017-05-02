#ifndef CASE_ARBUF
#define CASE_ARBUF

namespace CASE {

template<class T>
class ArrayBuffer {
    T * begin_a, * begin_b;
    bool flipbit = true;

public:
    ArrayBuffer(T * a, T * b) : begin_a(a), begin_b(b)
    {
    }

    inline T * current() {
        return flipbit ? begin_a : begin_b;
    }

    inline T * next() {
        return flipbit ? begin_b : begin_a;
    }

    inline T * flip() {
        flipbit = !flipbit;
        return current();
    }

    T & operator[](const int index) {
        return *(current() + index);
    }
};

}

#endif

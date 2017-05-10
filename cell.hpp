#ifndef CASE_CELL
#define CASE_CELL

#include "code.hpp"
#include "neighbors.hpp"

namespace CASE {

template<class T>
class CellInterface {
public:
    virtual Code insert(T * t)       = 0;
    virtual Code insert(T & t)       { return insert(&t); }
    virtual T * extract()            = 0;
    virtual T * get()                = 0;
    virtual void clear()             = 0;
    virtual int popcount()     const = 0;
    virtual bool is_empty()    const = 0;
    virtual bool is_occupied() const { return !is_empty(); }
    virtual ~CellInterface()         {}
};

template<class T, int SIZE>
class StackCell {
    static_assert(SIZE > 0, "StackCell SIZE must be > 0.");
    T * stack[SIZE];
    int size = 0;

public:
    CASE::Neighbors<StackCell<T, SIZE>> neighbors;

    StackCell() { clear(); }

    Code insert(T * t) {
        if (t == nullptr)
            return Code::Rejected;
        if (size == SIZE)
            return Code::Rejected;

        t->cell = this;
        stack[size++] = t;
        return Code::OK;
    }

    template<typename... Args>
    Code insert(T & t, Args & ... args) {
        if (insert(&t) == Code::OK)
            return insert(args...);
        else
            return Code::Rejected;
    }

    Code insert(T & t) {
        return insert(&t);
    }

    T * extract() {
        if (size == 0)
            return nullptr;

        auto t = stack[--size];
        t->cell = nullptr;
        return t;
    }

    T * get() {
        if (size == 0)
            return nullptr;
        else
            return stack[size - 1];
    }

    void clear() {
        while (extract() != nullptr);
    }

    int popcount() const { return size; }
    bool is_empty() const { return size == 0; }
    bool is_occupied() const { return !is_empty(); }
};

template<class T>
using SimpleCell = StackCell<T, 1>;

}

#endif // CELL

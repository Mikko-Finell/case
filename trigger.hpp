#ifndef CASE_TRIGGER
#define CASE_TRIGGER

#include <cassert>
#include <deque>
#include <algorithm>

namespace CASE {

class Trigger {
    std::deque<double> values;
    const double low, high;
    bool state = false;

public:
    Trigger(double mid, double tol, double size = 1, double init = 0)
        : low(mid - mid * tol), high(mid + mid * tol)
    {
        assert(size >= 1);
        for (auto i = 0; i < size; i++)
            values.push_back(init);
        update(init);
    }

    bool update(const double value) {
        values.push_front(value);
        values.pop_back();
        const auto tot_dt = std::accumulate(values.cbegin(), values.cend(), 0);
        const auto avg_dt = tot_dt / values.size();
        if (state && avg_dt < low)
            state = false;
        else if (avg_dt > high)
            state = true;
        return read_state();
    }

    inline bool read_state() const  {
        return state;
    }
};

}

#endif

#include <iostream>
#include <array>
#include "../random.hpp"

int main() {
    using namespace std;
    using namespace CASE;

    constexpr int range = 255;
    array<int, range > results;

    for (auto i = 0; i < range; i++) {
        results[i] = 0;
    }

    Cauchy rand;

    for (auto i = 0; i < range * 20; i++) {
        const auto index = rand(255/2, 5);
        if (index >= 0 && index < range)
            results[index] += 1;
    }

    for (auto i = 0; i < results.size(); i++) {
        cout << i << " ";
        if (i < 100)
            cout << " ";
        if (i < 10)
            cout << " ";
        cout << "|";

        const auto xs = results.at(i);
        for (auto i = 0; i < xs; i += 5)
            cout << "*";
        cout << endl;
    }
}

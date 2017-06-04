#include <iostream>
#include <array>

#define CASE_DETERMINISTIC
#include "../random.hpp"

int main() {
    using namespace std;
    using namespace CASE;

    for (int i = 0; i < 40; i++)
        cout << seed() << endl;
}

#include <vector>
#include <iostream>
#include "../pair.hpp"

struct Data {
    int i = 0;
    Data(const int a) : i{a} {}
};

int main() {
    CASE::Pair<Data> data{Data{1}, Data{2}};

    std::cout << data.current().i << std::endl;

    data.current().i = 0;

    std::cout << data.current().i << std::endl;
}

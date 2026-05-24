#include <iostream>
#include "math/math.h"

int main() {
    std::cout << "Hello, consumer CMake!" << std::endl;
    std::cout << "10 + 32 = " << add(10, 32) << "\n";
    std::cout << "6 * 9 = " << multiply(6, 9) << "\n";
    return 0;
}

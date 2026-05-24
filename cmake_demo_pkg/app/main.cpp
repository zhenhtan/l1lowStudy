#include <iostream>
#include "math/math.h"

int main() {
    int a = 6;
    int b = 7;
    std::cout << a << " + " << b << " = " << add(a, b) << "\n";
    std::cout << a << " * " << b << " = " << multiply(a, b) << "\n";
    return 0;
}

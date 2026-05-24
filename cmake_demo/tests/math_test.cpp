#include <cstdlib>
#include <iostream>

#include "math/math.h"

int main() {
    if (add(2, 3) != 5) {
        std::cerr << "add test failed\n";
        return EXIT_FAILURE;
    }

    if (multiply(4, 5) != 20) {
        std::cerr << "multiply test failed\n";
        return EXIT_FAILURE;
    }

    std::cout << "math_test passed\n";
    return EXIT_SUCCESS;
}

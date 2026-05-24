#include <iostream>
#include <string>

// 泛型函数：只要类型 T 支持 operator>，就能复用
template <typename T>
T MaxValue(const T& a, const T& b)
{
    return (a > b) ? a : b;
}

int main()
{
    std::cout << MaxValue(3, 7) << "\n";                       // int
    std::cout << MaxValue(2.5, 1.8) << "\n";                   // double
    std::cout << MaxValue(std::string("apple"), std::string("banana")) << "\n"; // string
    return 0;
}
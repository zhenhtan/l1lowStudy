#include <iostream>
#include <string>
#include <string_view>

std::string getHelloWorld() {
    std::string hello = "Hello, World!";
    return hello;
}
 
void testStringExample() {
    std::string str = getHelloWorld();
    std::cout << str << std::endl;
}

int main() {
    testStringExample();
    return 0;
}
#include <iostream>

class Base {
public:
    void print() {  // 注意：没有 virtual
        std::cout << "Base::print()\n";
    }
};

class Derived : public Base {
public:
    void print ()  {  // 与基类同名同参 -> 隐藏（不是重写多态）
        std::cout << "Derived::print()\n";
    }
};

int main() {
    Derived d;
    Base* pb = &d;

    d.print();    // Derived::print()
    pb->print();  // Base::print()  (因为 Base::print 不是 virtual)

    return 0;
}
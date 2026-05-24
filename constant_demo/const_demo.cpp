#include <iostream>
#include <string>

class User {
public:
    User(std::string name, int age) : name_(std::move(name)), age_(age) {}

    // 1) 返回值前的 const（对内置类型通常意义不大）
    const int getAgeValue() { return age_; }

    // 2) 返回“指向 const 的指针”：不能通过返回指针改内容
    const std::string* getNamePtr() const { return &name_; }

    // 3) 参数是 const 引用：函数内不能改入参，且避免拷贝
    void printWithPrefix(const std::string& prefix) const  {
        prefix="new prefix"; // 错误：不能修改 const 引用参数
        std::cout << prefix << name_ << ", age=" << age_ << "\n";
    }

    // 4) 成员函数末尾 const：承诺不修改对象状态（this 是 const User*）
    int age() const { return age_; }

    // 非 const 成员函数：可修改对象
    void grow() { ++age_; }

private:
    std::string name_;
    int age_;
};

int main() {
    User u("Alice", 18);
    u.printWithPrefix("[user] ");
    u.grow();
    //*u.getNamePtr() = "Alice Updated";

    const User cu("Bob", 20);
    std::cout << cu.age() << "\n";   // OK：const 成员函数可被 const 对象调用
    // cu.grow();                     // 错误：非 const 成员函数不能用于 const 对象
}
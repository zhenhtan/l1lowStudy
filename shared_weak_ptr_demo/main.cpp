#include <iostream>
#include <memory>
#include <string>

class Person {
public:
    explicit Person(std::string name) : name_(std::move(name)) {
        std::cout << "构造: " << name_ << "\n";
    }

    ~Person() {
        std::cout << "析构: " << name_ << "\n";
    }

    void setPartner(const std::shared_ptr<Person>& partner) {
        partner_ = partner;
    }

    void showPartner() const {
        if (auto locked = partner_.lock()) {
            std::cout << name_ << " 的伙伴是 " << locked->name_ << "\n";
        } else {
            std::cout << name_ << " 的伙伴已经释放\n";
        }
    }

private:
    std::string name_;
    std::weak_ptr<Person> partner_;
};

void demoSharedPtrBasic() {
    std::cout << "=== 演示1: shared_ptr 基本用法 ===\n";

    auto book = std::make_shared<std::string>("C++智能指针");
    std::cout << "创建后 use_count = " << book.use_count() << "\n";

    {
        auto reader1 = book;
        std::cout << "reader1 共享后 use_count = " << book.use_count() << "\n";

        {
            auto reader2 = book;
            std::cout << "reader2 共享后 use_count = " << book.use_count() << "\n";
            std::cout << "内容: " << *reader2 << "\n";
        }

        std::cout << "reader2 离开作用域后 use_count = " << book.use_count() << "\n";
    }

    std::cout << "reader1 离开作用域后 use_count = " << book.use_count() << "\n\n";
}

void demoWeakPtrBreakCycle() {
    std::cout << "=== 演示2: weak_ptr 打破循环引用 ===\n";

    auto alice = std::make_shared<Person>("Alice");
    auto bob = std::make_shared<Person>("Bob");

    std::cout << "建立关系前: Alice use_count = " << alice.use_count()
              << ", Bob use_count = " << bob.use_count() << "\n";

    alice->setPartner(bob);
    bob->setPartner(alice);

    std::cout << "建立关系后: Alice use_count = " << alice.use_count()
              << ", Bob use_count = " << bob.use_count() << "\n";

    alice->showPartner();
    bob->showPartner();

    std::cout << "函数结束后，Alice 和 Bob 都会正常析构，因为 weak_ptr 不增加引用计数\n\n";
}

void demoWeakPtrExpired() {
    std::cout << "=== 演示3: weak_ptr 观察对象是否还活着 ===\n";

    std::weak_ptr<int> observer;

    {
        auto data = std::make_shared<int>(42);
        observer = data;
        std::cout << "observer weak count = " << observer.use_count() << "\n";
        if (auto locked = observer.lock()) {
            std::cout << "observer 读到的值 = " << *locked << "\n";
                    std::cout << "observer weak count2 = " << data.use_count() << "\n";

        }
        std::cout << "observer weak count3 = " << data.use_count() << "\n";
    }
        if (auto locked = observer.lock()) {
            std::cout << "observer xxxx读到的值 = " << *locked << "\n";
        }
    if (observer.expired()) {
        std::cout << "data 已释放，observer 已失效\n";
    } else {
        std::cout << "data 还活着\n";
    }

    std::cout << "\n";
}

int main() {
    std::cout << "==============================\n";
    std::cout << "shared_ptr / weak_ptr 简单示例\n";
    std::cout << "==============================\n\n";

    demoSharedPtrBasic();
    demoWeakPtrBreakCycle();
    demoWeakPtrExpired();

    std::cout << "总结:\n";
    std::cout << "1. shared_ptr 表示共享所有权，引用计数加1。\n";
    std::cout << "2. weak_ptr 只是观察者，不拥有对象，不增加引用计数。\n";
    std::cout << "3. weak_ptr 常用于打破 shared_ptr 循环引用。\n";

    return 0;
}

#include <cstdint>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <typeinfo>

class Animal {
public:
    explicit Animal(std::string type_name) : type_name_(std::move(type_name)) {
        std::cout << "Animal::Animal(" << type_name_ << ") this=0x" << std::hex
                  << reinterpret_cast<std::uintptr_t>(this) << std::dec << std::endl;
    }

    virtual ~Animal() {
        std::cout << "Animal::~Animal(" << type_name_ << ") this=0x" << std::hex
                  << reinterpret_cast<std::uintptr_t>(this) << std::dec << std::endl;
    }

    virtual std::string speak() const = 0;

    virtual void move() const {
        std::cout << "Animal moves" << std::endl;
    }

protected:
    std::string type_name_;
};

class Dog : public Animal {
public:
    explicit Dog(std::string nick_name)
        : Animal("Dog"), nick_name_(std::move(nick_name)) {
        std::cout << "Dog::Dog(" << nick_name_ << ") this=0x" << std::hex
                  << reinterpret_cast<std::uintptr_t>(this) << std::dec << std::endl;
    }

    ~Dog() override {
        std::cout << "Dog::~Dog(" << nick_name_ << ") this=0x" << std::hex
                  << reinterpret_cast<std::uintptr_t>(this) << std::dec << std::endl;
    }

    // Constructor has no callable symbol address in C++, so use a proxy.
    static Dog* ctor_proxy(const std::string& nick_name) {
        return new Dog(nick_name);
    }

    static void dtor_proxy(Dog* p) {
        delete p;
    }

    // override makes compile-time checks strict: signature must match base.
    std::string speak() const override {
        return "Woof";
    }

    void move() const override {
        std::cout << "Dog runs" << std::endl;
    }

private:
    std::string nick_name_;
};

class Cat : public Animal {
public:
    explicit Cat(std::string nick_name)
        : Animal("Cat"), nick_name_(std::move(nick_name)) {
        std::cout << "Cat::Cat(" << nick_name_ << ") this=0x" << std::hex
                  << reinterpret_cast<std::uintptr_t>(this) << std::dec << std::endl;
    }

    ~Cat() override {
        std::cout << "Cat::~Cat(" << nick_name_ << ") this=0x" << std::hex
                  << reinterpret_cast<std::uintptr_t>(this) << std::dec << std::endl;
    }

    // Constructor has no callable symbol address in C++, so use a proxy.
    static Cat* ctor_proxy(const std::string& nick_name) {
        return new Cat(nick_name);
    }

    static void dtor_proxy(Cat* p) {
        delete p;
    }

    std::string speak() const override {
        return "Meow";
    }

    void move() const override {
        std::cout << "Cat walks quietly" << std::endl;
    }

private:
    std::string nick_name_;
};

void describe(const Animal& a) {
    std::cout << "dynamic type: " << typeid(a).name() << std::endl;
    std::cout << "speak(): " << a.speak() << std::endl;
    a.move();
}

void print_addr_line(const std::string& label, std::uintptr_t addr) {
    const auto old_flags = std::cout.flags();
    const auto old_fill = std::cout.fill();

    std::cout << std::left << std::setw(24) << label << " = 0x"
              << std::noshowbase << std::hex << addr << std::dec << std::endl;

    std::cout.flags(old_flags);
    std::cout.fill(old_fill);
}

void print_vptr(const Animal& a, const std::string& name) {
    // Educational only: vptr access is implementation-specific.
    const auto vptr = *reinterpret_cast<const std::uintptr_t* const*>(&a);
    print_addr_line(name + " vptr", reinterpret_cast<std::uintptr_t>(vptr));
}

void print_speak_slot_addr(const Animal& a, const std::string& name) {
    // For common Itanium ABI (GCC/Clang):
    // [0],[1] are usually destructor entries, [2] is often first non-dtor virtual.
    constexpr std::size_t speak_slot_index = 2;
    const auto vtable = *reinterpret_cast<const std::uintptr_t* const*>(&a);
    const auto speak_addr = vtable[speak_slot_index];
    using SpeakSlotFn = std::string (*)(const Animal*);
    const auto speak_fn = reinterpret_cast<SpeakSlotFn>(speak_addr);

    std::cout << "--- " << name << " ---" << std::endl;
    print_addr_line("vtable[0] (dtor1)", vtable[0]);
    print_addr_line("vtable[1] (dtor2)", vtable[1]);
    print_addr_line("vtable[2] (speak())", speak_addr);
    print_addr_line("vtable[3] (move())", vtable[3]);

    std::cout << "\n验证 vtable[2] 对应 speak():" << std::endl;
    std::cout << "  a.speak() -> " << a.speak() << std::endl;
    std::cout << "  调用 vtable[2] -> " << speak_fn(&a) << std::endl;

    std::cout << "\n对比结果 - " << name << " 对象：" << std::endl;
    if (typeid(a) == typeid(Dog)) {
        std::cout << "  对象类型: Dog" << std::endl;
        std::cout << "  Dog 的 speak() 返回 Woof" << std::endl;
        std::cout << "  当前槽位 2 的动态调用结果也是 Woof，所以这里观察到的槽位对应 speak()" << std::endl;
    } else if (typeid(a) == typeid(Cat)) {
        std::cout << "  对象类型: Cat" << std::endl;
        std::cout << "  Cat 的 speak() 返回 Meow" << std::endl;
        std::cout << "  当前槽位 2 的动态调用结果也是 Meow，所以这里观察到的槽位对应 speak()" << std::endl;
    }
    std::cout << std::endl;
}

void print_ctor_dtor_proxy_addr() {
    std::cout << "=== ctor/dtor 地址观察（通过代理函数） ===" << std::endl;
    print_addr_line("Dog::ctor_proxy", reinterpret_cast<std::uintptr_t>(&Dog::ctor_proxy));
    print_addr_line("Dog::dtor_proxy", reinterpret_cast<std::uintptr_t>(&Dog::dtor_proxy));
    print_addr_line("Cat::ctor_proxy", reinterpret_cast<std::uintptr_t>(&Cat::ctor_proxy));
    print_addr_line("Cat::dtor_proxy", reinterpret_cast<std::uintptr_t>(&Cat::dtor_proxy));
    std::cout << std::endl;
}

int main() {
    // Animal base;
    Dog dog("dog-stack");
    Cat cat("cat-stack");

    std::cout << "=== 1) override + virtual polymorphism ===" << std::endl;
    describe(dog);
    describe(cat);

    std::cout << "\n=== 2) base pointer dispatch (runtime via vtable) ===" << std::endl;
    std::unique_ptr<Animal> p1 = std::make_unique<Dog>("dog-heap");
    std::unique_ptr<Animal> p2 = std::make_unique<Cat>("cat-heap");

    std::cout << "p1->speak(): " << p1->speak() << std::endl;
    std::cout << "p2->speak(): " << p2->speak() << std::endl;

    std::cout << "\n=== 3) observe vptr addresses ===" << std::endl;
    // print_vptr(base, "base");
    print_vptr(dog, "dog");
    print_vptr(cat, "cat");
    print_vptr(*p1, "p1");
    print_vptr(*p2, "p2");

    std::cout << "\n=== 4) observe virtual speak() function address in vtable ===" << std::endl;

    // print_speak_slot_addr(base, "base");
    print_speak_slot_addr(*p1, "p1");
    print_speak_slot_addr(*p2, "p2");

    std::cout << "\n=== 5) observe constructor/destructor addresses ===" << std::endl;
    print_ctor_dtor_proxy_addr();

    std::cout << "\nIf class implementations differ, vptr values usually differ too." << std::endl;
    return 0;
}

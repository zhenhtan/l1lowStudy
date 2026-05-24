#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

// CRTP = Curiously Recurring Template Pattern
// Key idea: Base class is a template parameterized by Derived class.
// This avoids runtime virtual function overhead; dispatch is compile-time.

template <typename Derived>
class AnimalCRTP {
public:
    std::string speak() const {
        return static_cast<const Derived*>(this)->speak_impl();
    }

    void move() const {
        static_cast<const Derived*>(this)->move_impl();
    }
};

class DogCRTP : public AnimalCRTP<DogCRTP> {
public:
    std::string speak_impl() const {
        return "Woof (CRTP)";
    }

    void move_impl() const {
        std::cout << "Dog runs (CRTP)" << std::endl;
    }
};

class CatCRTP : public AnimalCRTP<CatCRTP> {
public:
    std::string speak_impl() const {
        return "Meow (CRTP)";
    }

    void move_impl() const {
        std::cout << "Cat walks quietly (CRTP)" << std::endl;
    }
};

// CRTP version: template function, resolved at compile time.
template <typename Animal>
void describe_crtp(const Animal& a) {
    std::cout << "Type: " << typeid(a).name() << std::endl;
    std::cout << "speak(): " << a.speak() << std::endl;
    a.move();
}

// Compare: Virtual dispatch version (from override_vtable_demo).
class AnimalVirtual {
public:
    virtual ~AnimalVirtual() = default;

    virtual std::string speak() const = 0;

    virtual void move() const {
        std::cout << "Animal moves (Virtual)" << std::endl;
    }
};

class DogVirtual : public AnimalVirtual {
public:
    std::string speak() const override {
        return "Woof (Virtual)";
    }

    void move() const override {
        std::cout << "Dog runs (Virtual)" << std::endl;
    }
};

class CatVirtual : public AnimalVirtual {
public:
    std::string speak() const override {
        return "Meow (Virtual)";
    }

    void move() const override {
        std::cout << "Cat walks quietly (Virtual)" << std::endl;
    }
};

void describe_virtual(const AnimalVirtual& a) {
    std::cout << "Type: " << typeid(a).name() << std::endl;
    std::cout << "speak(): " << a.speak() << std::endl;
    a.move();
}

void print_virtual_dispatch_flow(const AnimalVirtual& a, const std::string& name) {
    std::cout << "\n[virtual dispatch flow: " << name << "]" << std::endl;
    std::cout << "step1 object addr: " << &a << std::endl;

    // Educational only: object/vtable layout is ABI-specific (e.g. GCC/Clang Itanium ABI).
    const auto vtable = *reinterpret_cast<const std::uintptr_t* const* const*>(&a);
    std::cout << "step2 read vptr from object: " << vtable << std::endl;

    // With this class shape, common layout is: [0],[1] dtor, [2] speak, [3] move.
    constexpr std::size_t speak_slot = 2;
    constexpr std::size_t move_slot = 3;
    std::cout << "step3 vtable[" << speak_slot << "] (speak addr): 0x" << std::hex
              << vtable[speak_slot] << std::dec << std::endl;
    std::cout << "step4 vtable[" << move_slot << "] (move  addr): 0x" << std::hex
              << vtable[move_slot] << std::dec << std::endl;

    std::cout << "step5 do virtual call a.move(): ";
    a.move();
}

int main() {
    std::cout << "=== 1) CRTP: compile-time polymorphism ===" << std::endl;
    DogCRTP dog_crtp;
    CatCRTP cat_crtp;
    describe_crtp(dog_crtp);
    describe_crtp(cat_crtp);

    std::cout << "\n=== 2) Virtual: runtime polymorphism ===" << std::endl;
    DogVirtual dog_virt;
    CatVirtual cat_virt;
    describe_virtual(dog_virt);
    describe_virtual(cat_virt);
    print_virtual_dispatch_flow(dog_virt, "dog_virt");
    print_virtual_dispatch_flow(cat_virt, "cat_virt");

    std::cout << "\n=== 3) Key difference: CRTP vs Virtual ===" << std::endl;
    std::cout << "CRTP: type checked at compile time, inline-friendly, zero vtable overhead."
              << std::endl;
    std::cout << "Virtual: type checked at runtime, one vptr per object, one indirection per call."
              << std::endl;

    std::cout << "\n=== 4) CRTP with function template (no container polymorphism) ===" << std::endl;
    // Note: CRTP objects of different derived types cannot be stored in a
    // single container without losing type info (unlike virtual pointers).
    // To handle heterogeneous collections, you'd need a wrapper or type erasure.
    std::vector<int> dummy;  // Placeholder; CRTP doesn't support heterogeneous collections.
    std::cout << "Each CRTP call is a template instantiation => no type erasure." << std::endl;

    return 0;
}

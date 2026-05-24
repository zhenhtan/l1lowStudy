#include <iostream>
#include <iomanip>
#include <type_traits>

// ================================
// 第一部分：传统enum的缺点演示
// ================================

// 缺点1：弱作用域 - 枚举成员会污染父作用域
enum WeakScopeEnum {
    General,   // 这个General直接进入全局作用域
    Light,
    Medium,
    Heavy
};

// 如果再定义另一个enum，成员名冲突就会出错
// enum AnotherEnum {
//     General,  // ❌ 编译错误：重定义
//     Fire,
//     Water
// };

// ================================
// 第二部分：强类型枚举（enum class）的优势
// ================================

// 优势1：强作用域 - 枚举成员被限制在枚举作用域内
enum class WeaponType {
    General,      // 必须用 WeaponType::General 访问
    Pistol,
    MachineGun,
    Cannon
};

// 优势1扩展：可以有另一个enum class，成员名可以相同
enum class ElementType {
    General,      // 与上面WeaponType::General不冲突！
    Fire,
    Water,
    Earth
};

// 优势3：可以指定底层类型（默认是int）
enum class Rarity : unsigned char {  // 指定为uint8_t，节省存储空间
    Common = 0,
    Rare = 1,
    Epic = 2,
    Legendary = 3
};

enum class Priority : unsigned short {  // 指定为uint16_t
    Low = 0,
    Medium = 100,
    High = 200,
    Critical = 300
};

// ================================
// 应用示例：武器配置系统
// ================================

struct Weapon {
    WeaponType type;
    ElementType element;
    Rarity rarity;
    Priority priority;
    
    Weapon(WeaponType t, ElementType e, Rarity r, Priority p)
        : type(t), element(e), rarity(r), priority(p) {}
};

// 打印函数 - 演示强类型的特性
void printWeaponInfo(const Weapon& weapon) {
    // 优势2：无法隐式转换为int，必须显式转换
    std::cout << "\n[Weapon Information]\n";
    std::cout << "  Type: " << static_cast<int>(weapon.type) << " ";
    
    switch (weapon.type) {
        case WeaponType::General:      std::cout << "(General)"; break;
        case WeaponType::Pistol:       std::cout << "(Pistol)"; break;
        case WeaponType::MachineGun:   std::cout << "(MachineGun)"; break;
        case WeaponType::Cannon:       std::cout << "(Cannon)"; break;
    }
    
    std::cout << "\n  Element: " << static_cast<int>(weapon.element) << " ";
    switch (weapon.element) {
        case ElementType::General:     std::cout << "(General)"; break;
        case ElementType::Fire:        std::cout << "(Fire)"; break;
        case ElementType::Water:       std::cout << "(Water)"; break;
        case ElementType::Earth:       std::cout << "(Earth)"; break;
    }
    
    std::cout << "\n  Rarity: " << static_cast<int>(weapon.rarity)
              << " bytes(" << sizeof(weapon.rarity) << ")";
    std::cout << "\n  Priority: " << static_cast<int>(weapon.priority)
              << " bytes(" << sizeof(weapon.priority) << ")";
    std::cout << "\n";
}

// ================================
// 演示隐式转换的编译错误
// ================================

void demoImplicitConversionError() {
    std::cout << "\n=== 演示2：转换限制 ===\n";
    
    // ✅ 正确：显式类型转换
    int weaponTypeValue = static_cast<int>(WeaponType::Cannon);
    std::cout << "Cannon's value: " << weaponTypeValue << "\n";
    
    // ✅ 正确：比较操作
    if (WeaponType::Cannon == WeaponType::Cannon) {
        std::cout << "Cannon equals Cannon ✓\n";
    }
    WeaponType t = WeaponType::General;
    // ❌ 下面的代码如果取消注释会编译错误：
    // WeaponType t = 5;                    // 错误：不能从int隐式转换为WeaponType
    // int i = WeaponType::Pistol;          // 错误：不能隐式转换为int
    // if (WeaponType::General > 10) { }    // 错误：不能与int比较
    
    std::cout << "（以上隐式转换均编译错误，代码安全性更高！）\n";
}

// ================================
// 演示底层类型指定
// ================================

void demoUnderlyingType() {
    std::cout << "\n=== 演示3：底层类型指定 ===\n";
    
    std::cout << "Rarity底层类型: ";
    if (std::is_same_v<std::underlying_type_t<Rarity>, unsigned char>) {
        std::cout << "unsigned char";
    }
    std::cout << ", 占用字节数: " << sizeof(Rarity) << "\n";
    
    std::cout << "Priority底层类型: ";
    if (std::is_same_v<std::underlying_type_t<Priority>, unsigned short>) {
        std::cout << "unsigned short";
    }
    std::cout << ", 占用字节数: " << sizeof(Priority) << "\n";
    
    std::cout << "WeaponType底层类型（默认int）, 占用字节数: " << sizeof(WeaponType) << "\n";
}

// ================================
// 演示强作用域
// ================================

void demoStrongScope() {
    std::cout << "\n=== 演示1：强作用域 ===\n";
    
    // ✅ 强类型枚举必须用类名前缀访问
    WeaponType w1 = WeaponType::General;
    ElementType e1 = ElementType::General;  // 两个General可以共存！
    
    std::cout << "WeaponType::General = " << static_cast<int>(w1) << "\n";
    std::cout << "ElementType::General = " << static_cast<int>(e1) << "\n";
    std::cout << "✓ 同名成员在不同enum class中不冲突\n";
    
    // ❌ 下面的代码编译错误：
    // WeaponType w2 = General;  // 错误：General不在作用域内
    // auto x = General;         // 错误：General不存在
    
    std::cout << "（不加WeaponType::前缀会编译错误）\n";
}


int main() {
    std::cout << "╔════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║           C++11 强类型枚举（enum class）演示                    ║\n";
    std::cout << "╚════════════════════════════════════════════════════════════════╝\n";
    
    // ================================
    // 演示1：强作用域
    // ================================
    demoStrongScope();
    
    // ================================
    // 演示2：转换限制
    // ================================
    demoImplicitConversionError();
    
    // ================================
    // 演示3：底层类型指定
    // ================================
    demoUnderlyingType();
    
    // ================================
    // 演示4：实际应用
    // ================================
    std::cout << "\n=== 演示4：实际应用 - 武器配置系统 ===\n";
    
    // 创建几个武器对象，展示类型安全
    Weapon sword(
        WeaponType::Pistol,
        ElementType::Fire,
        Rarity::Legendary,
        Priority::Critical
    );
    
    Weapon gun(
        WeaponType::MachineGun,
        ElementType::Water,
        Rarity::Epic,
        Priority::High
    );
    
    Weapon cannon(
        WeaponType::Cannon,
        ElementType::Earth,
        Rarity::Rare,
        Priority::Medium
    );
    
    printWeaponInfo(sword);
    printWeaponInfo(gun);
    printWeaponInfo(cannon);
    
    // ================================
    // 演示5：对比传统enum
    // ================================
    std::cout << "\n=== 演示5：对比传统enum ===\n";
    std::cout << "传统enum弱作用域示例：\n";
    std::cout << "  General = " << General << "（可直接访问，污染全局作用域）\n";
    std::cout << "  Light = " << Light << "\n";
    std::cout << "  Medium = " << Medium << "\n";
    std::cout << "  Heavy = " << Heavy << "\n";
    
    // ✅ 传统enum可以隐式转换为int
    int weakValue = General;  // ✅ 编译通过，但这容易出错
    std::cout << "  int weakValue = General;  // ✅ 编译通过但不安全\n";
    
    std::cout << "\n强类型枚举更安全！\n";
    
    std::cout << "\n╔════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║                  总结：enum class的优势                        ║\n";
    std::cout << "├────────────────────────────────────────────────────────────────┤\n";
    std::cout << "│ 1. 强作用域: 成员名被限制，不污染父作用域空间                  │\n";
    std::cout << "│ 2. 转换限制: 不可与整型隐式相互转换，提高类型安全性           │\n";
    std::cout << "│ 3. 底层类型: 可显式指定(uint8_t/uint16_t等)，节省存储空间    │\n";
    std::cout << "╚════════════════════════════════════════════════════════════════╝\n";
    WeakScopeEnum t2 = General;
    return 0;
}

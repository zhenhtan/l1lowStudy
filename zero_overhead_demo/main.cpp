#include <iostream>
#include <vector>
/*
Zero-overhead（零开销）‌ 是一种系统或语言设计原则，核心思想是：‌不为未使用的功能付出任何运行时代价，且使用该功能时性能不低于手动优化的底层实现‌。

该概念在多个技术领域有不同体现，以下是主要应用场景：

C++ 中的 Zero-Overhead Abstraction（零开销抽象）
这是 C++ 的核心设计哲学之一，由 Bjarne Stroustrup 提出，包含两层含义：

‌不用不付费‌：若未使用某语言特性（如异常、RTTI、模板、虚函数等），编译器不会为其生成额外代码或消耗运行时资源。
‌用了不输手写‌：使用高级抽象（如 std::vector、模板、范围 for 循环）生成的机器码，效率不低于等价的手写 C 代码。
典型示例
std::array<int, 3> 编译后等价于原生数组 int‌‌:ml-citation{ref="3" appearance="aggregated" data="citationList"}，无额外开销 ‌‌
。
std::vector 同时提供 operator[]（无边界检查，高效）和 at()（带边界检查，安全），按需选择，避免强制开销 ‌‌
。
模板在编译期实例化，生成专用于具体类型的代码，无运行时多态成本 ‌‌
。
注意：异常处理是‌例外‌——即使未抛出异常，只要代码中存在 try/catch 或 throw，编译器仍需生成栈展开信息，带来一定开销 ‌‌
。

其他领域的 Zero-Overhead
‌Zero-Overhead Linux‌：指精简后的 Linux 发行版，移除不必要的后台服务，降低系统资源占用，提升性能与安全性 ‌‌
。
‌vLLM 的 Zero-Overhead Prefix Caching‌：前缀缓存命中与否对推理性能几乎无影响，实现“零额外延迟” ‌‌
。
‌AI 系统中的 Zero-Overhead 预热机制‌：通过指令缓存、内存映射等技术，在模型冷启动阶段几乎不增加推理延迟 ‌‌
。
总结
‌Zero-overhead ≠ 没有成本，而是“按需付费，用则最优”‌。它强调：

抽象不牺牲性能；
语义明确，代价透明；
编译器/系统协助实现高效，而非强制通用开销。
这一原则使 C++ 在保持高级抽象能力的同时，仍可用于系统编程、嵌入式、游戏引擎等对性能敏感的场景 ‌‌

*/
// Runtime polymorphism: indirect call through vtable.
struct IOp
{
    virtual ~IOp() = default;
    virtual int Apply(int x) const = 0;
};

struct AddOneVirtual : public IOp
{
    int Apply(int x) const override { return x + 1; }
};

int SumRuntime(const std::vector<int>& data, const IOp& op)
{
    int sum = 0;
    for (const int x : data)
    {
        sum += op.Apply(x);
    }
    return sum;
}

// Compile-time polymorphism: fully known type, can be inlined.
struct AddOneStatic
{
    int Apply(int x) const { return x + 1; }
};

template <typename Op>
int SumStatic(const std::vector<int>& data, const Op& op)
{
    int sum = 0;
    for (const int x : data)
    {
        sum += op.Apply(x);
    }
    return sum;
}

int main()
{
    const std::vector<int> data{1, 2, 3, 4, 5};

    AddOneVirtual runtimeOp;
    const int runtimeResult = SumRuntime(data, runtimeOp);

    AddOneStatic staticOp;
    const int staticResult = SumStatic(data, staticOp);

    std::cout << "runtime result: " << runtimeResult << '\n';
    std::cout << "static  result: " << staticResult << '\n';
    std::cout << "same output, different abstraction cost model" << '\n';

    return 0;
}

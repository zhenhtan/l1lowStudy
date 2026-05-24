# 实现重点特性

## 目的
该示例从语言语义到底层实现，讲解 `override/virtual/vtable` 的关系。

## 关键实现点
- 抽象基类 + 派生类覆盖，展示运行时多态调用行为。
- 使用 `override` 做签名一致性编译期校验。
- 通过基类指针/引用触发动态分发。
- 打印对象 `vptr` 地址做教学观察（实现相关，仅用于学习）。
- 读取典型 ABI 下的部分 vtable 槽位并对照调用结果。
- 对比不同动态类型对应函数地址与输出差异。

## 实践价值
- 帮助建立“高层多态语义”与“底层分发表机制”之间的对应认知。


在 Itanium ABI（GCC/Clang on Linux x86_64）下，虚表中的两个析构器槽是标准行为：

槽位	ABI 名称	触发场景
vtable[0]	D1 — complete-object destructor（完整对象析构器）	析构栈对象或基类子对象时调用，只析构，不 free

vtable[1]	D0 — deleting destructor（删除析构器）	delete ptr 时调用，析构 + operator delete 释放堆内存
所以你代码里执行 delete p1 时，编译器通过 vptr 跳到 vtable[1]（D0）；而栈对象 dog 作用域结束时，走的是 vtable[0]（D1）。

你可以用 nm 验证这两个地址对应的符号：
nm -n main | grep -E '_ZN3Dog|_ZN3Cat' | head -30

D1/D0 符号 mangling 规则：

_ZN3DogD1Ev → complete-object destructor（vtable[0]）
_ZN3DogD0Ev → deleting destructor（vtable[1]）
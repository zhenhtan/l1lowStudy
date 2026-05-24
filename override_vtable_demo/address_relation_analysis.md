# override_vtable_demo 地址关系分析（基于 2026-05-15 的一次运行输出）

## 1. 分析范围与前提

本分析严格基于你提供的这一轮 `./main` 输出。

- 地址值受 ASLR、装载基址、编译选项影响：跨次运行通常会变化。
- 但“同一次运行内的相对关系”通常稳定，适合观察 C++ 多态对象布局与虚表分发特征。

---

## 2. 对象 `this` 地址关系

### 2.1 栈对象

- `Dog dog("dog-stack")` 的 `this`：`0x7ffea2f67820`
- `Cat cat("cat-stack")` 的 `this`：`0x7ffea2f67870`
- 差值：
  - `0x7ffea2f67870 - 0x7ffea2f67820 = 0x50`

结论：本次运行中两个栈对象在栈帧内相距 `0x50` 字节。

### 2.2 堆对象

- `Dog dog-heap` 的 `this`：`0x55a7711ac2c0`
- `Cat cat-heap` 的 `this`：`0x55a7711ac310`
- 差值：
  - `0x55a7711ac310 - 0x55a7711ac2c0 = 0x50`

结论：本次堆分配也出现 `0x50` 字节间隔，和栈对象间距一致。这是当前对象大小/分配器状态下的观测结果，不是语言层保证。

---

## 3. vptr（虚表指针）关系

输出中：

- `dog vptr = 0x55a7410dbbf0`
- `p1 vptr  = 0x55a7410dbbf0`
- `cat vptr = 0x55a7410dbbc0`
- `p2 vptr  = 0x55a7410dbbc0`

结论：

1. 同动态类型共享同一 vtable：
   - `dog` 和 `p1`（都为 `Dog` 动态类型）vptr 完全一致。
   - `cat` 和 `p2`（都为 `Cat` 动态类型）vptr 完全一致。
2. 不同动态类型 vptr 不同：
   - `Dog vptr - Cat vptr = 0x30`。

这正是多态分发的核心：对象首部 vptr 指向所属动态类型的虚函数表。

---

## 4. vtable 槽位地址关系

### 4.1 Dog 对象（p1）

- `vtable[0] (dtor1) = 0x55a7410d60ec`
- `vtable[1] (dtor2) = 0x55a7410d61bc`
- `vtable[2] (speak) = 0x55a7410d62cc`
- `vtable[3] (move)  = 0x55a7410d6360`

槽位间差值：

- `vtable[1] - vtable[0] = 0xd0`
- `vtable[2] - vtable[1] = 0x110`
- `vtable[3] - vtable[2] = 0x94`

### 4.2 Cat 对象（p2）

- `vtable[0] (dtor1) = 0x55a7410d6550`
- `vtable[1] (dtor2) = 0x55a7410d6620`
- `vtable[2] (speak) = 0x55a7410d6730`
- `vtable[3] (move)  = 0x55a7410d67c4`

槽位间差值：

- `vtable[1] - vtable[0] = 0xd0`
- `vtable[2] - vtable[1] = 0x110`
- `vtable[3] - vtable[2] = 0x94`

### 4.3 Dog 与 Cat 的横向对比

每个对应槽位都满足：

- `Cat_slot - Dog_slot = 0x464`

即：

- `0x6550 - 0x60ec = 0x464`
- `0x6620 - 0x61bc = 0x464`
- `0x6730 - 0x62cc = 0x464`
- `0x67c4 - 0x6360 = 0x464`

结论：Dog/Cat 各虚函数实现在代码段中的布局呈现“整体平移”关系，这通常来自同一编译单元内符号排列与链接布局。

---

## 5. ctor/dtor 代理函数地址关系

输出中：

- `Dog::ctor_proxy = 0x55a7410d61eb`
- `Dog::dtor_proxy = 0x55a7410d62a1`
- `Cat::ctor_proxy = 0x55a7410d664f`
- `Cat::dtor_proxy = 0x55a7410d6705`

### 5.1 同类内差值

- `Dog::dtor_proxy - Dog::ctor_proxy = 0xb6`
- `Cat::dtor_proxy - Cat::ctor_proxy = 0xb6`

### 5.2 跨类平移

- `Cat::ctor_proxy - Dog::ctor_proxy = 0x464`
- `Cat::dtor_proxy - Dog::dtor_proxy = 0x464`

### 5.3 与 vtable 槽位的相对位置（Dog 为例）

- `vtable[1] (0x...61bc) < ctor_proxy (0x...61eb) < dtor_proxy (0x...62a1) < vtable[2] (0x...62cc)`

这说明“虚表中记录的函数入口地址”与“普通静态函数地址（代理函数）”都落在同一代码段内，且由链接器按一定顺序布局；但它们彼此的精确顺序不具备 C++ 标准语义保证。

---

## 6. 动态分发验证与地址关系闭环

你输出中两组验证都成功：

- p1：`a.speak() -> Woof`，`vtable[2] -> Woof`
- p2：`a.speak() -> Meow`，`vtable[2] -> Meow`

结论：在当前 ABI 与编译器下，观测到 `vtable[2]` 对应 `speak()`，并且对象的 vptr 决定了运行期调用哪个实现。

---

## 7. 析构阶段地址与顺序关系

末尾输出顺序：

1. 先析构堆对象（`p2` 再 `p1`，对应构造逆序）
2. 再析构栈对象（`cat` 再 `dog`，对应定义逆序）
3. 每个对象内部都表现为：
   - 派生析构先执行（`Cat::~Cat` / `Dog::~Dog`）
   - 基类析构后执行（`Animal::~Animal`）
4. 同一对象派生/基类析构打印的 `this` 地址完全一致。

这与 C++ 对象生命周期规则完全一致，也侧面验证了继承层级上的同一对象地址传递。

---

## 8. 总结

本次输出揭示了 4 个稳定事实：

1. 同动态类型对象共享同一 vptr，不同动态类型 vptr 不同。
2. `vtable[2]` 在这次构建中对应 `speak()`，与动态调用结果一致。
3. Dog 与 Cat 的虚函数地址、代理函数地址都呈现固定平移量 `0x464`，反映了代码段布局规律。
4. 析构顺序与 `this` 地址一致性符合 C++ 生命周期与继承析构规则。

如果你愿意，下一步可以再补一版“带符号反查”的分析：用 `nm -n`/`objdump -d` 把这些地址映射到具体符号名和汇编入口，形成地址到符号的一一对照表。

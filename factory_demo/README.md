# factory_demo 说明

这个示例对应你前面提到的两个 L1LOW 例子：

- 简单工厂：模仿 `HwConstantsApiFactory::CreateHwConstantsApi()`
- 抽象工厂：模仿 `IStaticHoCreator::MakeObjects()`

代码文件在 [main.cpp](/home/zhenhtan/workspace/l1low/leaningProgram/factory_demo/main.cpp)。

## 先看结论

两者最核心的区别只有一句话：

- 简单工厂：**根据输入，返回某一个具体产品对象**
- 抽象工厂：**提供一组创建接口，用来创建一类相关对象或对象集合**

放到这个例子里：

- 简单工厂关心的是“给我一个硬件类型，我返回一个对应的 `HwConstantsApi` 实现”
- 抽象工厂关心的是“给我一个 creator，这个 creator 负责生产某一类启动对象”

## main.cpp 逐段解释

### 1. 头文件部分

```cpp
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
```

这几行只是准备基础能力：

- `iostream` 用来打印结果
- `memory` 用来使用 `std::unique_ptr`
- `stdexcept` 用来抛异常
- `string` 和 `vector` 用来保存业务数据

这里最重要的是 `std::unique_ptr`，因为工厂模式通常会把“对象创建责任”封装起来，调用方拿到的是对象所有权，而不是对象具体构造细节。

---

### 2. 简单工厂命名空间

```cpp
namespace simple_factory {
```

这部分代码单独放在一个命名空间里，是为了和后面的抽象工厂示例隔离开，方便对比。

---

### 3. 抽象产品接口 `HwConstantsApi`

```cpp
class HwConstantsApi
{
public:
    virtual ~HwConstantsApi() = default;
    virtual auto GetBoardName() const -> std::string = 0;
    virtual auto GetMaxCarriers() const -> int = 0;
};
```

这就是“产品接口”。

含义是：

- 调用者不关心具体是 `MadeHwConstantsApi` 还是 `MarsHwConstantsApi`
- 调用者只关心“你是不是一个 `HwConstantsApi`”

这和 L1LOW 里的 `HwConstantsApi` 思路一致：

- 上层代码依赖接口
- 工厂决定返回哪个具体实现

---

### 4. 两个具体产品类

```cpp
class MadeHwConstantsApi : public HwConstantsApi
class MarsHwConstantsApi : public HwConstantsApi
```

这两个类就是“工厂真正创建出来的对象”。

它们分别实现相同接口，但返回不同数据：

- `MadeHwConstantsApi` 返回 `MADE` 和 `4`
- `MarsHwConstantsApi` 返回 `MARS` 和 `8`

这一步体现的是“同一个抽象接口，不同具体实现”。

---

### 5. 枚举 `HardwareVariant`

```cpp
enum class HardwareVariant
{
    Made,
    Mars
};
```

这是工厂的输入参数，也就是“选择条件”。

简单工厂最常见的写法就是：

- 输入一个类型
- 内部 `switch` / `if` 判断
- 返回对应对象

---

### 6. 简单工厂函数 `CreateHwConstantsApi`

```cpp
auto CreateHwConstantsApi(HardwareVariant variant) -> std::unique_ptr<HwConstantsApi>
```

这是整个简单工厂示例的核心。

它做了三件事：

1. 接收一个外部条件 `variant`
2. 根据 `variant` 选择具体实现类
3. 返回统一接口类型 `std::unique_ptr<HwConstantsApi>`

具体分支：

```cpp
case HardwareVariant::Made:
    return std::make_unique<MadeHwConstantsApi>();
case HardwareVariant::Mars:
    return std::make_unique<MarsHwConstantsApi>();
```

这里就是最标准的简单工厂：

- **调用者只调用一个工厂函数**
- **工厂函数只负责返回一个产品对象**

为什么叫“简单工厂”：

- 因为创建逻辑集中在一个函数里
- 类型一多，这个函数就会越来越长
- 它适合对象种类有限、创建逻辑不复杂的场景

---

### 7. 简单工厂的使用方式 `simple_factory::Run()`

```cpp
const auto api = CreateHwConstantsApi(HardwareVariant::Mars);
```

这一句最值得看。

调用方只知道：

- 我要一个 `Mars` 版本的能力对象

调用方不知道，也不需要知道：

- 是谁 new 的
- 具体类名是什么
- 构造时做了什么初始化

随后直接按接口调用：

```cpp
api->GetBoardName()
api->GetMaxCarriers()
```

这就是工厂模式的价值：**把“怎么创建”从“怎么使用”里拆开。**

---

## 再看抽象工厂部分

### 8. 抽象工厂命名空间

```cpp
namespace abstract_factory {
```

这一部分用来模拟 `IStaticHoCreator` 那种风格。

和前面的简单工厂不同，这里重点不是“返回某个单独对象”，而是“定义一组 creator，由不同 creator 生产不同类别的数据”。

---

### 9. 产品数据 `StartupObject`

```cpp
struct StartupObject
{
    std::string name;
    std::string payload;
};
```

这表示“被创建出来的对象内容”。

在真实 L1LOW 里，`IStaticHoCreator` 创建的是 HLAPI object；在这个 demo 里，为了更容易看懂，我把它简化成了一个普通结构体。

---

### 10. 事务类型 `TransactionType`

```cpp
enum class TransactionType
{
    CreateCarrier,
    CreateBeam
};
```

这个枚举表示“这个 creator 属于哪一类创建任务”。

对应关系是：

- `CarrierCreator` 负责 carrier 类对象
- `BeamCreator` 负责 beam 类对象

---

### 11. 抽象工厂接口 `IStaticHoCreator`

```cpp
class IStaticHoCreator
{
public:
    virtual ~IStaticHoCreator() = default;
    virtual auto MakeObjects() -> std::vector<std::unique_ptr<StartupObject>> = 0;
    virtual auto GetTransactionType() const -> TransactionType = 0;
};
```

这里是整个抽象工厂示例的核心。

它不像简单工厂那样只暴露一个“创建对象的全局函数”，而是先定义了一类“creator 接口”。

这个接口规定，每个 creator 都必须回答两个问题：

- 你能创建什么类型的事务对象？
- 你创建出来的对象集合是什么？

这就是抽象工厂的特点：

- **先定义工厂接口**
- **再由不同具体工厂实现这套接口**
- **每个工厂负责一个产品家族，或者一组相互相关的对象**

---

### 12. 具体工厂 `CarrierCreator`

```cpp
class CarrierCreator : public IStaticHoCreator
```

它实现了两个接口函数：

```cpp
auto MakeObjects() -> std::vector<std::unique_ptr<StartupObject>> override
auto GetTransactionType() const -> TransactionType override
```

含义是：

- 这个 creator 专门负责创建 carrier 相关对象
- 一次可以返回多个对象

你可以把它理解成“某一类对象的专属工厂”。

---

### 13. 具体工厂 `BeamCreator`

```cpp
class BeamCreator : public IStaticHoCreator
```

它和 `CarrierCreator` 的结构一样，但负责的是另一组对象。

这一步体现了抽象工厂的扩展性：

- 想增加新对象家族，不需要改调用方主流程
- 新增一个 creator 类，实现相同接口即可

这和简单工厂很不一样。

如果用简单工厂，通常要去改那个 `switch`；
如果用抽象工厂，很多时候只要新增一个具体工厂类，再把它注册进集合里。

---

### 14. 工厂集合 `CreateStartupCreators()`

```cpp
auto CreateStartupCreators() -> std::vector<std::unique_ptr<IStaticHoCreator>>
```

这一段很关键。

它返回的不是产品，而是“一组工厂对象”：

```cpp
creators.push_back(std::make_unique<CarrierCreator>());
creators.push_back(std::make_unique<BeamCreator>());
```

这就是简单工厂和抽象工厂最明显的区别之一：

- 简单工厂返回的是“产品”
- 抽象工厂更常见的是返回“工厂接口集合”或者“某个具体工厂”，再由工厂去生产产品家族

---

### 15. `ToString()`

```cpp
auto ToString(TransactionType type) -> const char*
```

这个函数只是为了打印更直观，和设计模式本身关系不大。

---

### 16. 抽象工厂的使用方式 `abstract_factory::Run()`

```cpp
auto creators = CreateStartupCreators();
for (const auto& creator : creators)
{
    std::cout << "transaction=" << ToString(creator->GetTransactionType()) << "\n";
    auto objects = creator->MakeObjects();
    for (const auto& object : objects)
    {
        ...
    }
}
```

这段代码体现了抽象工厂的典型调用方式：

- 先拿到一组“工厂”
- 再逐个工厂去生成属于自己的对象集合

也就是说，调用方流程是：

- 我不直接关心 `CarrierCreator` 细节
- 我只知道它们都实现了 `IStaticHoCreator`
- 所以我可以统一遍历、统一调用

这非常适合启动阶段、配置装配阶段、插件式扩展场景。

---

### 17. `main()`

```cpp
int main()
{
    try
    {
        simple_factory::Run();
        abstract_factory::Run();
    }
    catch (const std::exception& ex)
    {
        ...
    }
}
```

这里把两个示例放在同一个程序里顺序执行，方便你直接比较输出结果。

---

## 最核心的区别

### 简单工厂

代码特征：

- 一个工厂函数
- 一个输入参数
- 根据条件返回一个具体产品

适合场景：

- 产品种类不多
- 分支逻辑简单
- 只需要“选一个对象出来”

这个 demo 对应：

- `CreateHwConstantsApi(HardwareVariant variant)`

一句话总结：

- **简单工厂解决的是“我该 new 哪个具体类”。**

### 抽象工厂

代码特征：

- 先定义工厂接口
- 不同具体工厂实现这套接口
- 每个工厂负责一组相关对象或同类对象集合

适合场景：

- 有多类相关对象要一起创建
- 希望新增种类时少改主流程
- 希望通过统一接口组织不同 creator

这个 demo 对应：

- `IStaticHoCreator`
- `CarrierCreator`
- `BeamCreator`

一句话总结：

- **抽象工厂解决的是“我如何组织多种创建器，去生成相关的一组对象”。**

---

## 和 L1LOW 的映射关系

### 对应 `HwConstantsApiFactory`

L1LOW 里：

- 根据平台/硬件环境
- 返回一个 `HwConstantsApi` 实现

这和 demo 里的：

- `CreateHwConstantsApi(HardwareVariant::Made/Mars)`

是同一思路。

### 对应 `IStaticHoCreator`

L1LOW 里：

- 不同 creator 负责构造不同静态 HLAPI objects
- 主流程只依赖统一接口 `IStaticHoCreator`

这和 demo 里的：

- `CarrierCreator`
- `BeamCreator`
- `CreateStartupCreators()`

是同一思路。

---

## 怎么判断该用哪一种

可以用一个简单标准：

- 如果你只是“在多个具体类里选一个返回”，优先用简单工厂
- 如果你需要“定义一套创建接口，并让多个具体工厂各自负责一个对象家族”，用抽象工厂

换成更直白的话：

- **选对象**：简单工厂
- **组织一组创建器**：抽象工厂

---

## 运行方式

```bash
cd /home/zhenhtan/workspace/l1low/leaningProgram/factory_demo
cmake -S . -B build
cmake --build build
./build/factory_demo
```

如果你愿意，下一步我可以继续把这个 demo 再升级成两版：

1. 拆成 `simple_factory/` 和 `abstract_factory/` 两个子目录，结构更清晰
2. 再补一个“工厂方法（Factory Method）”版本，顺手把它和简单工厂、抽象工厂三者区别讲清楚
# Python 面试重点示例

这个目录提供可直接运行的最简代码，覆盖常见 Python 面试考点。

## 目录说明

- 01_basics.py: 基础语法、推导式、集合与切片
- 02_oop.py: 类、封装、异常与 __repr__
- 03_functional.py: 装饰器、闭包
- 04_iter_gen.py: 迭代器与生成器
- 05_asyncio_demo.py: asyncio 并发基础
- 06_algo.py: two-sum、括号匹配
- 07_threading_vs_multiprocessing.py: 多线程与 GIL、进程并行对比
- 08_copy_memory.py: 浅拷贝/深拷贝与可变默认参数陷阱
- 09_common_interview_scenarios.py: 补充高频点（`is None`、[]/None 三态、`sorted` vs `sort`、`lru_cache`、dataclass、EAFP、keyword-only、JSON 解析异常）
- 10_class_members.py: 普通 class 常见成员（类属性、实例属性、实例/类/静态方法、property、魔术方法、`__dict__`）
- 11_typing_examples.py: `typing` 竖切示例（网关 Literal、可观测性、埋点窗口 TypeVar、对账 Protocol、UserId Annotated、BFF `Awaitable` 聚合）
- run_all.py: 一键运行全部示例

## 运行方式

在当前目录执行:

```bash
python3 run_all.py
```

也可以单独运行任一文件，例如:

```bash
python3 03_functional.py
```

## 其他常见高频（可与官方文档对照）

| 主题 | 现实关联举例 |
|------|----------------|
| `super()` 与 MRO | 多继承下插件链调用父类同一方法 |
| `__enter__` / `__exit__` | 自研资源：DB 事务、临时目录、锁 |
| `nonlocal` / `global` | 闭包里累加计数器 vs 模块级配置 |
| 列表推导式作用域 | 推导式内 `:=` 与海象运算符可读性 |
| `import` 与循环依赖 | 包初始化顺序、延迟 import |
| `__hash__` 与可变对象 | dict/set 的 key 为何必须不可变 |
| `weakref` | 缓存大对象又不想阻止 GC |
| `typing.Protocol` | 结构化子类型（duck typing 正式化） |
| `contextlib.contextmanager` | 用生成器写 `with` 块 |
| `concurrent.futures` | 线程池封装 HTTP 批量回调 |



## 常见数据类型可以分成几类来看，下面给你一个实用版清单。

数值类型
int：整数
场景：计数、索引、ID、循环次数、订单数量
float：浮点数
场景：温度、金额计算中的近似值、评分、比例
complex：复数
场景：科学计算、信号处理（工程里偶尔用）
布尔类型
bool：True 或 False
场景：条件判断、开关状态、权限控制、过滤逻辑
文本类型
str：字符串
场景：姓名、地址、日志、接口参数、JSON 字段值、正则处理
序列类型
list：可变有序序列
场景：任务队列、收集查询结果、需要增删改的集合
tuple：不可变有序序列
场景：函数返回多个值、固定配置项、作为字典键
range：整数序列生成器
场景：for 循环计数、按区间遍历
映射类型
dict：键值对映射
场景：配置对象、JSON 解析、按键快速查找、数据聚合统计
集合类型
set：无序不重复集合
场景：去重、成员测试、集合运算（交集并集差集）
frozenset：不可变集合
场景：需要可哈希集合、作为字典键、只读集合配置
二进制类型
bytes：不可变字节序列
场景：网络协议报文、文件二进制读取、加密数据
bytearray：可变字节序列
场景：高频修改的二进制缓冲区
memoryview：内存视图
场景：大块二进制数据零拷贝切片，提高性能
空类型
NoneType（值是 None）
场景：表示无值、默认参数占位、函数尚无结果
其他常见“容器语义”类型（标准库）
deque（双端队列）
场景：消息队列、滑动窗口、头尾高效插入删除
Counter
场景：词频统计、元素计数
defaultdict
场景：分组聚合，避免手动判空初始化
NamedTuple 或 dataclass
场景：结构化业务数据，代码可读性高于纯字典
实战选型建议

需要顺序且常改：list
需要键值组织：dict
需要去重或集合运算：set
数据不应被修改：tuple 或 frozenset
接口返回“可能没有值”：None
处理网络包或文件流：bytes 或 bytearray
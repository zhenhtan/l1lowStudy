"""Python 面试重点 03: 装饰器、闭包与函数式写法"""

import time
from typing import Callable, Any
from decimal import Decimal

def timing(func: Callable[..., Any]) -> Callable[..., Any]:
    # 装饰器本质: 接收函数并返回新函数
    def wrapper(*args: Any, **kwargs: Any) -> Any:
        start = time.perf_counter()
        result = func(*args, **kwargs)
        end = time.perf_counter()
        print(f"{func.__name__} cost: {end - start:.6f}s")
        return result

    return wrapper


@timing # 语法糖: 等价于 slow_sum = timing(slow_sum)
def slow_sum(n: int) -> int:
    return sum(range(n + 1))


def closure_demo() -> Callable[[int], int]:
    # -----------------------------------------------------------------------
    # 场景 1: 函数工厂 — 用同一套逻辑生成"行为不同"的函数
    # 场景: 不同税率的计税器，税率只在构造时固定，不需要每次传参
    # -----------------------------------------------------------------------
    def make_tax_calculator(rate: float) -> Callable[[float], int]: # 闭包工厂: 接收参数，返回一个函数
        def calc(price: float) -> int: # 内部函数: 捕获外部 rate，不同实例各自独立, 返回 int 类型的价格去小数
            return int(Decimal(price) * (Decimal(1) + Decimal(rate)))        # 捕获外部 rate，不同实例各自独立
        return calc

    cn_tax = make_tax_calculator(0.13)       # 国内 13% 税率
    eu_tax = make_tax_calculator(0.20)       # 欧盟 20% 税率
    print("函数工厂 — 商品 100 元:")
    print(f"  CN 税后: {cn_tax(100):.2f}")
    print(f"  EU 税后: {eu_tax(100):.2f}")

    # -----------------------------------------------------------------------
    # 场景 2: 私有状态计数器 — 不用 class 也能封装可变状态
    # 场景: 每次调用都累加，但状态对外不可见，也不污染全局变量
    # -----------------------------------------------------------------------
    def make_counter(start: int = 0) -> Callable[[], int]:
        print(f"创建计数器，初始值: {start}")
        count = [start]
        def increment():
            #nonlocal count  # 告诉 Python: count 是外层变量
            count[0] += 1
            print(f"计数器状态: {count[0]}")  # 展示每次调用的状态变化
            return count[0]
        def decrement():  #只做展示用，实际不调用
            count[0] -= 1
            print(f"计数器状态: {count[0]}")  # 展示每次调用的状态变化
            return count[0]
        print("make_counter 内部函数 increment 已创建，准备返回")
        return increment

    counter_a = make_counter()  # counter_a 即是 increment 函数，且捕获了 count 这个列表作为私有状态
    counter_b = make_counter(100)            # 两个计数器互不干扰
    print("\n私有状态计数器:")
    print(f"  counter_a: {counter_a()}, {counter_a()}, {counter_a()}")
    print(f"  counter_b: {counter_b()}, {counter_b()}")

    # -----------------------------------------------------------------------
    # 场景 3: for 循环捕获陷阱 — 闭包捕获的是"变量引用"而非"当前值"
    # -----------------------------------------------------------------------
    # 错误写法: 所有函数都共享同一个 i，循环结束后 i=2，全部打印 2
    wrong_funcs = [lambda: i for i in range(3)]

    # 正确写法: 用默认参数把当前值"快照"进去，或者用闭包工厂捕获当时的值
    def make_printer(n: int) -> Callable[[], int]:
        def printer() -> int:
            return n                         # n 在 make_printer 调用时就固定了
        return printer

    right_funcs = [make_printer(i) for i in range(3)]

    print("\nfor 循环捕获陷阱:")
    print(f"  错误写法结果: {[f() for f in wrong_funcs]}")   # [2, 2, 2]
    print(f"  正确写法结果: {[f() for f in right_funcs]}")   # [0, 1, 2]


def functional_demo() -> None:
    print("slow_sum:", slow_sum(100000))
    print("\n===== 闭包三大使用场景 =====")
    closure_demo()


if __name__ == "__main__":
    functional_demo()
    print("\n结论: 装饰器和闭包是 Python 函数式编程的核心工具，掌握它们能写出更简洁、灵活的代码。")
    count = [10]  # 用列表模拟可变整数
    print(count)  # 输出 [10]
    print(count[0])  # 输出 10

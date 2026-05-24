"""Python 面试重点 04: 迭代器与生成器"""

from typing import Iterator


def fibonacci(limit: int) -> Iterator[int]:
    # 生成器: 按需产出，节省内存
    a, b = 0, 1
    for _ in range(limit):
        yield a  # 产出当前值，函数状态冻结，保存当前的a, 等待下一次迭代
        a, b = b, a + b
        print(f"生成器状态: a={a}, b={b}")  # 展示每次迭代的状态变化
    print("生成器结束")  # 迭代完成后的提示


class CountDown:
    # 自定义可迭代对象: 实现 __iter__
    def __init__(self, n: int) -> None:
        self.n = n

    def __iter__(self) -> Iterator[int]:
        current = self.n
        while current > 0:
            yield current+3  # 模拟一些计算，展示迭代器的状态变化, 保存current的状态到yield中
            current -= 1
            print(f"CountDown 迭代状态: current={current}")  # 展示每次迭代的状态变化
        print("CountDown 结束")  # 迭代完成后的提示

def iter_gen_demo() -> None:
    print("fibonacci(8):", list(fibonacci(8)))
    print("countdown from 5:", list(CountDown(5)))


if __name__ == "__main__":
    iter_gen_demo()

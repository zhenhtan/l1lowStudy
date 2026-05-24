"""Python 面试重点 01: 基础语法与数据结构"""


def basics_demo() -> None:
    # 列表推导式: 常见高频考点
    nums = [1, 2, 3, 4, 5]
    squares = [n * n for n in nums]

    # 字典推导式: 反转映射或构造索引
    square_map = {n: n * n for n in nums}

    # 集合: 自动去重
    dedup = set([1, 1, 2, 3, 3])

    # 切片: 左闭右开
    part = nums[1:4]

    print("squares:", squares)
    print("square_map:", square_map)
    print("dedup:", dedup)
    print("slice 1:4:", part)


if __name__ == "__main__":
    basics_demo()

"""Python 面试重点 06: 常见算法题 — 真实应用场景版"""


# -----------------------------------------------------------------------
# 场景 1: 凑单优惠（哈希表 O(n)）
#
# 背景: 电商平台"任意两件商品总价等于满减门槛即可享受折扣"
#       给定商品价格列表和目标金额，找出能凑单的两件商品
# -----------------------------------------------------------------------
def find_discount_pair(
    prices: list[tuple[str, int]], target: int
) -> tuple[str, str] | None:  # tuple[str, str] | None 表示返回值可以是两个字符串的元组或None
    # key=价格, value=商品名
    seen: dict[int, str] = {}
    for item, price in prices:
        need = target - price
        if need in seen:
            print(f"找到凑单组合: {item} ({price} 元) + {seen[need]} ({need} 元) = {target} 元")
            return (seen[need], item)   # 找到凑单组合
        seen[price] = item
        print(f"当前商品: {item} ({price} 元), 需要凑单: {need} 元, 已见过的价格: {list(seen.keys())}")
    return None


# -----------------------------------------------------------------------
# 场景 2: JSON/代码格式校验（栈结构）
#
# 背景: 编辑器或 CI 流水线检查配置文件括号是否配对
#       常见于 JSON、Python、C++ 等代码的语法预检
# -----------------------------------------------------------------------
def is_brackets_balanced(code: str) -> bool:
    pair = {")": "(", "]": "[", "}": "{"}
    stack: list[str] = []
    for ch in code:
        if ch in "([{":
            stack.append(ch)
            print(f"入栈: {ch}, 当前栈: {stack}")
        elif ch in ")]}":
            if not stack or stack[-1] != pair[ch]:
                print(f"括号不匹配: {ch}, 当前栈: {stack}")
                return False            # 括号不匹配，语法错误
            print(f"出栈: {ch}, 当前栈: {stack}")
            stack.pop()
            print(f"出栈: {ch}, 当前栈: {stack}")
    return len(stack) == 0             # 栈为空表示所有括号都配对了


def algo_demo() -> None:
    # 场景 1: 商品凑单
    products = [
        ("iPhone 壳", 29),
        ("数据线", 49),
        ("耳机", 121),
        ("充电头", 71),
    ]
    target_price = 120   # 满 120 元享折扣

    result = find_discount_pair(products, target_price)
    if result:
        print(f"凑单成功: {result[0]} + {result[1]} = {target_price} 元，享折扣！")
    else:
        print("没有找到能凑单的两件商品")

    # 场景 2: 代码括号校验
    snippets = [
        ('{"name": "alice", "scores": [90, 95]}',  True),   # 合法 JSON
        ('def foo(x): return (x + 1',               False),  # 缺少右括号
        ('for i in range(len(arr)):\n    pass',     True),   # 合法 Python
        ('[1, 2, (3 + 4])',                          False),  # 括号嵌套错误
    ]

    print()
    for code, expected in snippets:
        print(f"代码: {code}")
        ok = is_brackets_balanced(code)
        status = "✓ 合法" if ok else "✗ 语法错误"
        assert ok == expected, f"结果不符预期: {code}"
        print(f"  {status}: {code[:45]}")


if __name__ == "__main__":
    algo_demo()

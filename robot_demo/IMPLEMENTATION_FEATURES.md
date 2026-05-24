# 实现重点特性

## 目的
该模块展示如何把 Python 函数暴露为 Robot Framework 关键字。

## 关键实现点
- 使用 `robot.api.deco.keyword` 声明可被 Robot 用例调用的关键字。
- `Add Numbers` 实现参数转整型后的可复用算术逻辑。
- `Join With Dash` 提供简单字符串拼接关键字。
- 关键字函数保持无状态，保证测试行为可预测。

## 实践价值
- 给出 Robot + Python 扩展的最小实现路径，便于快速扩展自定义测试能力。

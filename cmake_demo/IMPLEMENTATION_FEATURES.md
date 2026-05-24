# 实现重点特性

## 目的
该示例展示一个最小但完整的 CMake 工程拆分方式：库、应用、测试与安装规则。

## 关键实现点
- 将 `math/math.cpp` 构建为静态库 `math_lib`，对外通过 `math/math.h` 提供接口。
- 使用 `target_include_directories(... PUBLIC ...)` 传播头文件目录，体现正确的 usage requirements。
- 可执行程序 `demo` 通过链接库复用能力，而不是把全部源码混编在一个目标里。
- 通过 `ENABLE_WARNINGS` 开关统一控制告警策略。
- 通过 `include(CTest)` 和 `add_test` 集成单元测试。
- 使用 `GNUInstallDirs` 组织可执行文件、库文件与头文件安装布局。

## 实践价值
- 体现清晰的目标边界和依赖关系。
- 可直接作为多目标 CMake 项目的起步模板。

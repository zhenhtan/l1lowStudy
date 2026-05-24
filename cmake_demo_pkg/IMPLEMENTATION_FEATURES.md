# 实现重点特性

## 目的
该示例在基础 CMake 工程之上，进一步演示“可安装、可发现、可复用”的包化能力。

## 关键实现点
- 保留模块化结构：`math_lib` 库、`demo` 程序、可选测试。
- 提供别名目标 `MathLib::math_lib`，让使用方链接语义更清晰。
- 使用 `BUILD_INTERFACE/INSTALL_INTERFACE` 区分构建期与安装期头文件路径。
- 导出 `MathLibTargets`，生成可供外部项目导入的 targets 文件。
- 使用 `configure_package_config_file` 与 `write_basic_package_version_file` 生成配置和版本文件。
- 支持消费方通过 `find_package(MathLib)` 接入。

## 实践价值
- 演示从“本地工程构建”到“可分发 CMake 包”的关键升级路径。

这种方式的作用是:
consumer_project只需要:
find_package(MathLib REQUIRED CONFIG)
(未指定版本号，所以任何版本都可接受）
或者
find_package(MathLib 1.0.0 REQUIRED CONFIG)
(只接受1.0.0)
不需要指定头文件目录:
target_include_directories(consumer_demo PRIVATE
    "${MATH_LIB_ROOT}/include"
)

结论：在 l1low 主工程里，find_package 用得不少，但“标准包导出流程（configure_package_config_file + write_basic_package_version_file + install(EXPORT)）”并没有大量使用。主工程以“消费包（find_package）”为主，不是“大量自建并导出标准 CMake 包”。
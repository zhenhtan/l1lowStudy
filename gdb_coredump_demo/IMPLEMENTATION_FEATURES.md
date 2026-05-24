# 实现重点特性

## 目的
该示例用于构造可重复崩溃场景，演练 core dump 与 gdb 事后分析流程。

## 关键实现点
- 在 `process_user` 中故意解引用空指针，制造确定性崩溃。
- 保留清晰调用链（`main -> handle_request -> process_user`）以便回溯定位。
- 编译参数使用 `-g3 -O0 -fno-omit-frame-pointer -rdynamic`，提升可调试性。
- 支持拆分调试符号：生成 `.debug` 文件并剥离运行时二进制。
- 通过 GNU debuglink 关联符号文件，模拟线上“剥离包 + 符号包”场景。
- 用最小 C 程序聚焦崩溃分析主线，避免无关复杂度。

## 实践价值
- 接近真实生产排障流程，便于练习“复现-抓取-core-符号化-定位”闭环。

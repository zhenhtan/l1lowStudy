# LeaningProgram Markdown Viewer

一个轻量本地网页工具，用来浏览 `leaningProgram/` 目录中的 Markdown 文档。

## 功能

- 自动扫描 `leaningProgram` 下所有 `.md` 文件
- 自动过滤 `.venv`、`node_modules`、`build` 等目录
- 左侧文件搜索 + 右侧实时渲染
- 支持标题、代码块、列表、引用、表格、链接等常见 Markdown 语法
- 手机和桌面端都可用

## 启动

```bash
cd leaningProgram/md_web_viewer
python3 server.py
```

启动后访问：

http://127.0.0.1:8765

## 说明

- 服务只允许读取 `leaningProgram` 目录内 `.md` 文件。
- 如果新增了 Markdown 文件，点击页面右上角“刷新列表”即可更新。

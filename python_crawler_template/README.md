# 合法合规 Python 爬虫最小模板

这个模板包含：
- requests 抓取网页
- BeautifulSoup 解析内容
- schedule 定时任务
- SQLite 入库
- robots.txt 合规检查（默认开启）

## 1. 安装

```bash
cd leaningProgram/python_crawler_template
python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
```

## 2. 配置

```bash
cp .env.example .env
# 修改 TARGET_URL、CSS_SELECTOR、抓取周期等
```

注意：
- 请先阅读目标站点服务条款与 robots.txt。
- 不要抓取个人隐私、受版权保护且无授权的数据。
- 建议保留真实可联系的 User-Agent。

## 3. 运行

```bash
python crawler.py
```

## 4. 查看数据

```bash
sqlite3 data/crawler.db "SELECT id, url, substr(extracted_text,1,80), fetched_at FROM crawl_records ORDER BY id DESC LIMIT 10;"
```

## 5. 扩展建议

- 增加字段：标题、作者、发布时间、原文链接
- 增加去重：对 URL+内容哈希建立唯一索引
- 增加重试与限速：指数退避、随机抖动
- 增加告警：连续失败触发邮件/IM 提醒

import logging
import os
import sqlite3
import sys
import time
from datetime import datetime, timezone
from pathlib import Path
from urllib.parse import urlparse
from urllib.robotparser import RobotFileParser

import requests
import schedule
from bs4 import BeautifulSoup
from dotenv import load_dotenv


def setup_logging() -> None:
    logging.basicConfig(
        level=logging.INFO,
        format="%(asctime)s | %(levelname)s | %(message)s",
        handlers=[logging.StreamHandler(sys.stdout)],
    )


def env_bool(name: str, default: bool) -> bool:
    value = os.getenv(name)
    if value is None:
        return default
    return value.strip().lower() in {"1", "true", "yes", "y", "on"}


def load_config() -> dict:
    load_dotenv()

    url = os.getenv("TARGET_URL", "https://example.com/").strip()
    selector = os.getenv("CSS_SELECTOR", "h1").strip()
    every_minutes = int(os.getenv("CRAWL_EVERY_MINUTES", "1"))
    user_agent = os.getenv(
        "USER_AGENT", "CompliantCrawler/1.0 (+contact: your_email@example.com)"
    ).strip()
    timeout = int(os.getenv("REQUEST_TIMEOUT", "15"))
    check_robots = env_bool("CHECK_ROBOTS", True)
    db_path = os.getenv("DB_PATH", "data/crawler.db").strip()

    return {
        "url": url,
        "selector": selector,
        "every_minutes": every_minutes,
        "user_agent": user_agent,
        "timeout": timeout,
        "check_robots": check_robots,
        "db_path": db_path,
    }


def ensure_db(db_path: str) -> None:
    db_file = Path(db_path)
    db_file.parent.mkdir(parents=True, exist_ok=True)

    schema_path = Path(__file__).with_name("schema.sql")
    schema_sql = schema_path.read_text(encoding="utf-8")

    with sqlite3.connect(db_file) as conn:
        conn.executescript(schema_sql)
        conn.commit()


def is_allowed_by_robots(url: str, user_agent: str) -> bool:
    parsed = urlparse(url)
    robots_url = f"{parsed.scheme}://{parsed.netloc}/robots.txt"

    rp = RobotFileParser()
    rp.set_url(robots_url)
    try:
        rp.read()
        return rp.can_fetch(user_agent, url)
    except Exception as exc:
        logging.warning("读取 robots.txt 失败: %s；默认拒绝抓取。", exc)
        return False


def extract_text(html: str, selector: str) -> str:
    soup = BeautifulSoup(html, "html.parser")
    nodes = soup.select(selector)
    print(f"找到 {len(nodes)} 个节点匹配 selector='{selector}'")
    for node in nodes:
        print(f"节点文本预览: {node.get_text(' ', strip=True)[:100]}")
    if not nodes:
        return ""
    return " | ".join(node.get_text(" ", strip=True) for node in nodes)


def save_record(db_path: str, url: str, selector: str, extracted_text: str) -> None:
    fetched_at = datetime.now(timezone.utc).isoformat()
    with sqlite3.connect(db_path) as conn:
        conn.execute(
            """
            INSERT INTO crawl_records(url, selector, extracted_text, fetched_at)
            VALUES (?, ?, ?, ?)
            """,
            (url, selector, extracted_text, fetched_at),
        )
        conn.commit()


def crawl_once(cfg: dict) -> None:
    url = cfg["url"]
    selector = cfg["selector"]
    user_agent = cfg["user_agent"]
    timeout = cfg["timeout"]
    db_path = cfg["db_path"]

    if cfg["check_robots"] and not is_allowed_by_robots(url, user_agent):
        logging.error("robots.txt 不允许抓取: %s", url)
        return

    headers = {"User-Agent": user_agent}

    try:
        resp = requests.get(url, headers=headers, timeout=timeout)
        resp.raise_for_status()
    except requests.RequestException as exc:
        logging.error("请求失败: %s", exc)
        return

    text = extract_text(resp.text, selector)
    if not text:
        logging.warning("未匹配到任何内容，selector=%s", selector)
        return

    save_record(db_path, url, selector, text)
    logging.info("抓取成功并入库: %s", text[:120])


def main() -> None:
    setup_logging()
    cfg = load_config()

    ensure_db(cfg["db_path"])

    # 启动时先执行一次，便于快速验证。
    crawl_once(cfg)

    every_minutes = cfg["every_minutes"]
    if every_minutes <= 0:
        raise ValueError("CRAWL_EVERY_MINUTES 必须大于 0")

    schedule.every(every_minutes).minutes.do(crawl_once, cfg=cfg)
    logging.info("定时任务已启动：每 %s 分钟抓取一次", every_minutes)

    while True:
        schedule.run_pending()
        time.sleep(1)


if __name__ == "__main__":
    main()

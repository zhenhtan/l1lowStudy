CREATE TABLE IF NOT EXISTS crawl_records (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    url TEXT NOT NULL,
    selector TEXT NOT NULL,
    extracted_text TEXT NOT NULL,
    fetched_at TEXT NOT NULL
);

CREATE INDEX IF NOT EXISTS idx_crawl_records_url_time
ON crawl_records(url, fetched_at);

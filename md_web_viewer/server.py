#!/usr/bin/env python3
"""Local markdown viewer for files under leaningProgram.

Run:
  python3 server.py
Then open http://127.0.0.1:8765
"""

from __future__ import annotations

import json
import os
from http import HTTPStatus
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from urllib.parse import parse_qs, unquote, urlparse

HOST = "127.0.0.1"
PORT = 8765

APP_ROOT = Path(__file__).resolve().parent


def resolve_md_root() -> Path:
    # Priority: explicit env var -> repository interview dir -> legacy parent dir.
    env_root = os.getenv("MD_VIEWER_ROOT", "").strip()
    if env_root:
        return Path(env_root).expanduser().resolve()

    interview_root = APP_ROOT.parents[1] / "interview"
    if interview_root.exists() and interview_root.is_dir():
        return interview_root.resolve()

    return APP_ROOT.parent.resolve()


MD_ROOT = resolve_md_root()

IGNORED_DIRS = {
    ".git",
    ".idea",
    ".vscode",
    ".venv",
    "venv",
    "node_modules",
    "__pycache__",
    "build",
    "dist",
    "out",
}

STATIC_FILES = {
    "/": "index.html",
    "/index.html": "index.html",
    "/app.js": "app.js",
    "/styles.css": "styles.css",
}


def _is_within(child: Path, parent: Path) -> bool:
    try:
        child.resolve().relative_to(parent.resolve())
        return True
    except ValueError:
        return False


def list_markdown_files() -> list[dict[str, str]]:
    items: list[dict[str, str]] = []
    for root, dirs, files in os.walk(MD_ROOT):
        dirs[:] = [
            d
            for d in dirs
            if d not in IGNORED_DIRS and not d.startswith(".") and "venv" not in d.lower()
        ]

        root_path = Path(root)
        for name in files:
            if not name.lower().endswith(".md"):
                continue

            file_path = root_path / name
            if not _is_within(file_path, MD_ROOT):
                continue

            rel = file_path.relative_to(MD_ROOT).as_posix()
            items.append(
                {
                    "path": rel,
                    "name": name,
                }
            )

    return sorted(items, key=lambda x: x["path"].lower())


class MarkdownViewerHandler(BaseHTTPRequestHandler):
    server_version = "LeaningProgramMarkdownViewer/1.0"

    def do_GET(self) -> None:
        parsed = urlparse(self.path)

        if parsed.path == "/api/list":
            self._handle_api_list()
            return

        if parsed.path == "/api/file":
            self._handle_api_file(parsed.query)
            return

        self._serve_static(parsed.path)

    def _handle_api_list(self) -> None:
        files = list_markdown_files()
        payload = {
            "root": MD_ROOT.name,
            "count": len(files),
            "files": files,
        }
        self._send_json(payload)

    def _handle_api_file(self, query: str) -> None:
        params = parse_qs(query)
        requested = params.get("path", [""])[0]
        requested = unquote(requested).strip()

        if not requested:
            self._send_json({"error": "Missing path query"}, status=HTTPStatus.BAD_REQUEST)
            return

        target = (MD_ROOT / requested).resolve()

        if not _is_within(target, MD_ROOT):
            self._send_json({"error": "Path is outside allowed directory"}, status=HTTPStatus.FORBIDDEN)
            return

        if target.suffix.lower() != ".md":
            self._send_json({"error": "Only .md files are supported"}, status=HTTPStatus.BAD_REQUEST)
            return

        if not target.exists() or not target.is_file():
            self._send_json({"error": "File not found"}, status=HTTPStatus.NOT_FOUND)
            return

        text = target.read_text(encoding="utf-8", errors="replace")
        payload = {
            "path": target.relative_to(MD_ROOT).as_posix(),
            "content": text,
            "size": target.stat().st_size,
        }
        self._send_json(payload)

    def _serve_static(self, path: str) -> None:
        mapped = STATIC_FILES.get(path)
        if not mapped:
            self.send_error(HTTPStatus.NOT_FOUND, "Not Found")
            return

        file_path = APP_ROOT / mapped
        if not file_path.exists():
            self.send_error(HTTPStatus.NOT_FOUND, "Static file missing")
            return

        content_type = "text/plain; charset=utf-8"
        if file_path.suffix == ".html":
            content_type = "text/html; charset=utf-8"
        elif file_path.suffix == ".js":
            content_type = "application/javascript; charset=utf-8"
        elif file_path.suffix == ".css":
            content_type = "text/css; charset=utf-8"

        data = file_path.read_bytes()
        self.send_response(HTTPStatus.OK)
        self.send_header("Content-Type", content_type)
        self.send_header("Content-Length", str(len(data)))
        self.end_headers()
        self.wfile.write(data)

    def _send_json(self, payload: dict, status: HTTPStatus = HTTPStatus.OK) -> None:
        data = json.dumps(payload, ensure_ascii=False).encode("utf-8")
        self.send_response(status)
        self.send_header("Content-Type", "application/json; charset=utf-8")
        self.send_header("Content-Length", str(len(data)))
        self.end_headers()
        self.wfile.write(data)


def main() -> None:
    server = ThreadingHTTPServer((HOST, PORT), MarkdownViewerHandler)
    print(f"Markdown viewer started at http://{HOST}:{PORT}")
    print(f"Scanning markdown files under: {MD_ROOT}")
    print("Press Ctrl+C to stop.")
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        pass
    finally:
        server.server_close()
        print("Markdown viewer stopped.")


if __name__ == "__main__":
    main()

const state = {
  files: [],
  filtered: [],
  activePath: "",
};

const els = {
  searchInput: document.getElementById("searchInput"),
  refreshBtn: document.getElementById("refreshBtn"),
  fileTree: document.getElementById("fileTree"),
  fileCount: document.getElementById("fileCount"),
  docTitle: document.getElementById("docTitle"),
  docMeta: document.getElementById("docMeta"),
  renderArea: document.getElementById("renderArea"),
  copyPathBtn: document.getElementById("copyPathBtn"),
};

function escapeHtml(text) {
  return text
    .replaceAll("&", "&amp;")
    .replaceAll("<", "&lt;")
    .replaceAll(">", "&gt;")
    .replaceAll('"', "&quot;")
    .replaceAll("'", "&#39;");
}

function renderInline(text) {
  let out = escapeHtml(text);
  out = out.replace(/`([^`]+)`/g, "<code>$1</code>");
  out = out.replace(/\*\*([^*]+)\*\*/g, "<strong>$1</strong>");
  out = out.replace(/\*([^*]+)\*/g, "<em>$1</em>");
  out = out.replace(/\[([^\]]+)\]\(([^)]+)\)/g, '<a href="$2" target="_blank" rel="noreferrer">$1</a>');
  return out;
}

function parseTable(lines, startIndex) {
  const tableStart = lines[startIndex];
  const alignLine = lines[startIndex + 1];
  if (!tableStart || !alignLine) {
    return null;
  }

  const maybeHeader = tableStart.includes("|");
  const maybeAlign = /^\s*\|?\s*:?[-]{3,}:?\s*(\|\s*:?[-]{3,}:?\s*)+\|?\s*$/.test(alignLine);
  if (!maybeHeader || !maybeAlign) {
    return null;
  }

  const rows = [];
  let i = startIndex;
  while (i < lines.length && lines[i].includes("|")) {
    const raw = lines[i].trim().replace(/^\|/, "").replace(/\|$/, "");
    rows.push(raw.split("|").map((x) => x.trim()));
    i += 1;
  }

  if (rows.length < 2) {
    return null;
  }

  const header = rows[0];
  const body = rows.slice(2);

  let html = "<table><thead><tr>";
  for (const cell of header) {
    html += `<th>${renderInline(cell)}</th>`;
  }
  html += "</tr></thead><tbody>";

  for (const row of body) {
    html += "<tr>";
    for (const cell of row) {
      html += `<td>${renderInline(cell)}</td>`;
    }
    html += "</tr>";
  }
  html += "</tbody></table>";

  return {
    html,
    nextIndex: i,
  };
}

function renderMarkdown(mdText) {
  const lines = mdText.replaceAll("\r\n", "\n").split("\n");
  const blocks = [];

  let i = 0;
  while (i < lines.length) {
    const line = lines[i];

    if (!line.trim()) {
      i += 1;
      continue;
    }

    if (line.startsWith("```")) {
      const lang = line.slice(3).trim();
      const buf = [];
      i += 1;
      while (i < lines.length && !lines[i].startsWith("```")) {
        buf.push(lines[i]);
        i += 1;
      }
      i += 1;
      blocks.push(`<pre><code class="lang-${escapeHtml(lang)}">${escapeHtml(buf.join("\n"))}</code></pre>`);
      continue;
    }

    const table = parseTable(lines, i);
    if (table) {
      blocks.push(table.html);
      i = table.nextIndex;
      continue;
    }

    if (/^#{1,6}\s+/.test(line)) {
      const level = line.match(/^#+/)[0].length;
      const text = line.replace(/^#{1,6}\s+/, "");
      blocks.push(`<h${level}>${renderInline(text)}</h${level}>`);
      i += 1;
      continue;
    }

    if (/^>\s?/.test(line)) {
      const quote = [];
      while (i < lines.length && /^>\s?/.test(lines[i])) {
        quote.push(lines[i].replace(/^>\s?/, ""));
        i += 1;
      }
      blocks.push(`<blockquote>${renderInline(quote.join("<br />"))}</blockquote>`);
      continue;
    }

    if (/^\s*[-*+]\s+/.test(line)) {
      const list = [];
      while (i < lines.length && /^\s*[-*+]\s+/.test(lines[i])) {
        list.push(lines[i].replace(/^\s*[-*+]\s+/, ""));
        i += 1;
      }
      blocks.push(`<ul>${list.map((x) => `<li>${renderInline(x)}</li>`).join("")}</ul>`);
      continue;
    }

    if (/^\s*\d+\.\s+/.test(line)) {
      const list = [];
      while (i < lines.length && /^\s*\d+\.\s+/.test(lines[i])) {
        list.push(lines[i].replace(/^\s*\d+\.\s+/, ""));
        i += 1;
      }
      blocks.push(`<ol>${list.map((x) => `<li>${renderInline(x)}</li>`).join("")}</ol>`);
      continue;
    }

    if (/^\s*---+\s*$/.test(line)) {
      blocks.push("<hr />");
      i += 1;
      continue;
    }

    const para = [line.trim()];
    i += 1;
    while (
      i < lines.length &&
      lines[i].trim() &&
      !/^#{1,6}\s+/.test(lines[i]) &&
      !lines[i].startsWith("```") &&
      !/^>\s?/.test(lines[i]) &&
      !/^\s*[-*+]\s+/.test(lines[i]) &&
      !/^\s*\d+\.\s+/.test(lines[i])
    ) {
      para.push(lines[i].trim());
      i += 1;
    }
    blocks.push(`<p>${renderInline(para.join(" "))}</p>`);
  }

  return blocks.join("\n");
}

function setRenderHtml(html) {
  els.renderArea.classList.remove("fade-in");
  els.renderArea.innerHTML = html;
  requestAnimationFrame(() => {
    els.renderArea.classList.add("fade-in");
  });
}

function renderFileList() {
  els.fileCount.textContent = String(state.filtered.length);

  if (!state.filtered.length) {
    els.fileTree.innerHTML = "<p class='placeholder'>未找到匹配文件</p>";
    return;
  }

  const html = state.filtered
    .map((item) => {
      const isActive = item.path === state.activePath ? "active" : "";
      return `<button class="file-item ${isActive}" data-path="${escapeHtml(item.path)}" title="${escapeHtml(
        item.path
      )}">${escapeHtml(item.path)}</button>`;
    })
    .join("");

  els.fileTree.innerHTML = html;

  for (const node of els.fileTree.querySelectorAll(".file-item")) {
    node.addEventListener("click", () => {
      const path = node.getAttribute("data-path");
      if (path) {
        openFile(path);
      }
    });
  }
}

function applyFilter() {
  const keyword = els.searchInput.value.trim().toLowerCase();
  if (!keyword) {
    state.filtered = [...state.files];
  } else {
    state.filtered = state.files.filter((f) => f.path.toLowerCase().includes(keyword));
  }
  renderFileList();
}

async function loadList() {
  const res = await fetch("/api/list");
  if (!res.ok) {
    throw new Error(`加载文件列表失败: ${res.status}`);
  }

  const data = await res.json();
  state.files = data.files || [];
  state.filtered = [...state.files];
  renderFileList();
}

async function openFile(path) {
  state.activePath = path;
  renderFileList();

  const res = await fetch(`/api/file?path=${encodeURIComponent(path)}`);
  if (!res.ok) {
    setRenderHtml(`<p>读取失败：${res.status}</p>`);
    return;
  }

  const data = await res.json();
  const html = renderMarkdown(data.content || "");
  setRenderHtml(html || "<p class='placeholder'>空文档</p>");

  els.docTitle.textContent = data.path || path;
  const lines = (data.content || "").split("\n").length;
  els.docMeta.textContent = `${Math.round((data.size || 0) / 1024)} KB · ${lines} lines`;
}

async function copyActivePath() {
  if (!state.activePath) {
    return;
  }

  try {
    await navigator.clipboard.writeText(state.activePath);
    const oldText = els.copyPathBtn.textContent;
    els.copyPathBtn.textContent = "已复制";
    setTimeout(() => {
      els.copyPathBtn.textContent = oldText;
    }, 900);
  } catch (_e) {
    els.copyPathBtn.textContent = "复制失败";
  }
}

function bindEvents() {
  els.searchInput.addEventListener("input", applyFilter);
  els.refreshBtn.addEventListener("click", async () => {
    await loadList();
  });
  els.copyPathBtn.addEventListener("click", copyActivePath);
}

async function bootstrap() {
  bindEvents();
  try {
    await loadList();
    const first = state.filtered[0];
    if (first) {
      await openFile(first.path);
    }
  } catch (err) {
    const msg = err instanceof Error ? err.message : "加载失败";
    setRenderHtml(`<p>${escapeHtml(msg)}</p>`);
  }
}

bootstrap();

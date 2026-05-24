"""Python 面试补充 09: 更多高频点 + 贴近业务的短场景"""

from __future__ import annotations

import json
import tempfile
from dataclasses import dataclass
from functools import lru_cache
from pathlib import Path


# ---------------------------------------------------------------------------
# 1) `is None` 而不是 `== None`（身份 vs 可能被重载的相等）
#    场景: 依赖第三方/遗留类时，`__eq__` 写得离谱会导致 `== None` 误判
# ---------------------------------------------------------------------------
def scenario_is_none() -> None:
    class BadEq:
        def __eq__(self, other: object) -> bool:  # noqa: ARG002
            return True

    bad = BadEq()
    print(f"bad: {bad}")
    print(f"bad.__eq__(None): {bad.__eq__(None)}")
    print("1) is None vs == None")
    print("   bad == None →", bad == None)  # 这里不会报错，因为 __eq__ 返回 True
    print("   bad is None →", bad is None)  # 跟 __eq__ 不同，is 是判断对象身份，而不是相等性
    print("   结论: 判空用 `is None` / `is not None`，不用 ==。")


# ---------------------------------------------------------------------------
# 2) 空列表 [] 与 None 三态（REST / GraphQL 常见）
#    场景: 分页接口 — None=未请求, []=确实没有数据, [..]=有数据
# ---------------------------------------------------------------------------
def scenario_empty_vs_none() -> None:
    def fetch_orders(user_id: int) -> list[str] | None:
        if user_id < 0:
            return None  # 参数非法，表示「没有有效响应」
        if user_id == 0:
            return []  # 合法用户，确实 0 单
        return ["ORD-1", "ORD-2"]

    for uid, label in [(-1, "非法用户"), (0, "新用户"), (42, "老用户")]:
        rows = fetch_orders(uid)
        if rows is None:
            status = "打回请求 / 记监控"
        elif not rows:
            status = "展示「暂无订单」空状态页"
        else:
            status = f"渲染 {len(rows)} 条"
        print(f"2) {label}: rows={rows!r} → {status}")
    print("   结论: `if not rows` 会把 None 和 [] 混在一起；先判 is None。")


# ---------------------------------------------------------------------------
# 3) sorted 返回新列表 vs list.sort 原地改（报表共享引用踩坑）
#    场景: 多租户仪表盘共用同一份「原始 metrics」列表做排序展示
# ---------------------------------------------------------------------------
def scenario_sorted_vs_sort() -> None:
    base = [("tenant_b", 30), ("tenant_a", 10), ("tenant_c", 20)]
    for_display = sorted(base, key=lambda x: x[1])
    print("3) sorted 返回新列表 → 原始 base 不变:", base)
    print("   for_display:", for_display)
    base.sort(key=lambda x: x[1])
    print("   若误用 .sort() 且 base 还被「审计快照」引用 → 原始顺序被破坏:", base)
    print("   结论: 要保留原顺序时用 sorted(...) 或先 copy 再 sort。")


# ---------------------------------------------------------------------------
# 4) functools.lru_cache — 场景: 网关对同一 user_id 反复查权限/配额
# ---------------------------------------------------------------------------
@lru_cache(maxsize=256)
def load_role_permissions(user_id: int) -> frozenset[str]:
    # 假装查 DB / LDAP，很贵
    return frozenset({f"perm:{user_id}", "read:invoice"})


def scenario_lru_cache() -> None:
    uid = 9001
    a = load_role_permissions(uid)
    b = load_role_permissions(uid)
    print("4) lru_cache 命中同一对象?", a is b, "| cache_info:", load_role_permissions.cache_info())


# ---------------------------------------------------------------------------
# 5) dataclass — 场景: 从 CSV/消息队列反序列化一行计费记录
# ---------------------------------------------------------------------------
@dataclass(slots=True)
class UsageRecord:
    tenant: str
    cpu_seconds: int
    billable: bool = True


def scenario_dataclass() -> None:
    row = UsageRecord("acme", 3600)
    print("5) dataclass:", row, "| slots 省内存、字段名有补全/类型提示。")
    #print(f"row: {row.__dict__}")
    print(f"row: {row.__slots__}") # 只显示 slots 定义的字段
    print(f"row: {row.__getattribute__('tenant')}") # 获取 tenant 字段的值
    print(f"row: {row.__getattribute__('cpu_seconds')}") # 获取 cpu_seconds 字段的值
    print(f"row: {row.__getattribute__('billable')}") # 获取 billable 字段的值    


# ---------------------------------------------------------------------------
# 6) EAFP vs LBYL — 场景: 部署脚本读配置，竞态下 exists 不可靠
# ---------------------------------------------------------------------------
def scenario_eafp(tmp: Path) -> None:
    cfg = tmp / "app.toml"
    cfg.write_text("env = \"prod\"\n", encoding="utf-8")
    # LBYL: if cfg.exists(): open — 中间可能被删；EAFP: 直接 try
    try:
        text = cfg.read_text(encoding="utf-8")
        print("6) EAFP read_text 成功，首行:", text.splitlines()[0])
    except OSError as e:
        print("读失败:", e)


# ---------------------------------------------------------------------------
# 7) 仅限关键字参数 — 场景: 公共 SDK 加参数不破坏老代码位置实参
# ---------------------------------------------------------------------------
def emit_event(name: str, *, payload: dict | None = None, dry_run: bool = False) -> str:
    if dry_run:
        return f"[dry-run] {name} {payload!r}"
    return f"sent {name}"


def scenario_kw_only() -> None:
    print("7) keyword-only:", emit_event("order_paid", payload={"id": 1}, dry_run=True))
    print("   `*` 后参数不能用位置传入，避免以后在中间插入新参数时调用方错位。")


# ---------------------------------------------------------------------------
# 8) json.loads + 具体异常 — 场景: Webhook 体不是合法 JSON
# ---------------------------------------------------------------------------
def scenario_json_eafp(raw: str) -> None:
    try:
        data = json.loads(raw)
    except json.JSONDecodeError as e:
        print("8) JSONDecodeError:", e.msg, "at pos", e.pos)
        return
    print("8) parsed keys:", list(data))


def main() -> None:
    scenario_is_none()
    scenario_empty_vs_none()
    scenario_sorted_vs_sort()
    scenario_lru_cache()
    scenario_dataclass()
    with tempfile.TemporaryDirectory() as td:
        scenario_eafp(Path(td))
    scenario_kw_only()
    scenario_json_eafp("{not json")


if __name__ == "__main__":
    main()

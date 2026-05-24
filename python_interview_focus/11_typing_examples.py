"""Python 面试补充 11: typing 常见构造 + 贴近业务的可运行脚本

涵盖: Awaitable、Protocol、TypeVar（约束）、Literal、Annotated。
"""

from __future__ import annotations

import asyncio
from typing import (
    Annotated,
    Awaitable,
    Literal,
    Protocol,
    TypeVar,
    cast,
    get_args,
    get_origin,
    runtime_checkable,
)

# ---------------------------------------------------------------------------
# TypeVar 约束：同一「指标批次」里时间戳必须是同一标量类型（int epoch 或 ISO str）
# 场景: 埋点 / 计费聚合服务把「窗口起止」成对校验
# ---------------------------------------------------------------------------
Ts = TypeVar("Ts", int, str)


def merge_same_type_window(start: Ts, end: Ts) -> tuple[Ts, Ts]:
    if type(start) is not type(end):
        raise TypeError("同一窗口起止必须是同一类型（int epoch 或 str ISO）")
    if start > end:
        raise ValueError("start after end")
    return (start, end)


# ---------------------------------------------------------------------------
# Protocol：计费流水线里「只要能导出 CSV 行」就接入，不依赖具体供应商类
# ---------------------------------------------------------------------------
@runtime_checkable
class InvoiceExporter(Protocol):
    def to_csv_row(self) -> str: ...


class LegacyErpAdapter:
    """模拟老 ERP：没有继承任何 Protocol，只实现同名方法。"""

    def __init__(self, invoice_no: int) -> None:
        self._invoice_no = invoice_no

    def to_csv_row(self) -> str:
        return f"ERP,{self._invoice_no},posted"


class StripeLikeAdapter:
    """模拟 Stripe 类 SaaS：结构同上，可进同一流水线。"""

    def __init__(self, charge_id: str) -> None:
        self._charge_id = charge_id

    def to_csv_row(self) -> str:
        return f"STRIPE,{self._charge_id},captured"


def append_to_daily_export(row: InvoiceExporter) -> None:
    print("  写入日终文件:", row.to_csv_row())


# ---------------------------------------------------------------------------
# Literal：可观测性 + 网关
# ---------------------------------------------------------------------------
LogLevel = Literal["DEBUG", "INFO", "WARN", "ERROR"]


def emit_structured(level: LogLevel, service: str, msg: str) -> dict[str, str]:
    return {"level": level, "service": service, "msg": msg}


IngressPort = Literal[80, 443]


def bind_socket_hint(port: IngressPort) -> str:
    """入口只监听 80/443 时，运维脚本生成 nginx listen 片段。"""
    return "listen 443 ssl;" if port == 443 else "listen 80;"


# ---------------------------------------------------------------------------
# Annotated：ORM / API 层在类型上挂「列语义」，运行时校验器读取元数据（FastAPI/Pydantic 同款思路）
# ---------------------------------------------------------------------------
UserId = Annotated[int, "users.id", "positive"] # Annotated 是一个类型，表示一个类型加上一个或多个元数据


def validate_user_id(uid: int) -> UserId:
    """校验通过后，用 cast 告诉类型检查器：此 int 已满足 UserId 约定。

    运行时 `UserId` 仍是普通 int；`Annotated` 不会自动变类型，`cast` 只影响静态检查。
    """
    if uid <= 0:
        raise ValueError("UserId Annotated 约定: 必须为正整数")
    return cast(UserId, uid)


def describe_user(uid: UserId) -> str:   
    return f"https://internal.example/users/{uid}" # 返回用户ID的URL


def explain_annotated_schema() -> None:
    origin = get_origin(UserId)
    args = get_args(UserId)
    print("  get_origin(UserId) →", origin, "| get_args →", args)
    print("  UserId:", UserId.__class__)
    print("  UserId.__name__:", UserId.__name__)
    print("  UserId.__module__:", UserId.__module__)
    print("  UserId.__qualname__:", UserId.__qualname__)
    print("  UserId.__metadata__:", UserId.__metadata__)


# ---------------------------------------------------------------------------
# Awaitable：编排层同时拉多个下游（用户标签、配额），不关心具体是协程还是 Task
# ---------------------------------------------------------------------------
async def fetch_user_tier(uid: int) -> str:
    await asyncio.sleep(0)
    return "enterprise" if uid % 2 == 0 else "free"


async def fetch_quota_headroom(uid: int) -> int:
    await asyncio.sleep(0)
    return 10_000 if uid == 501 else 100


async def dashboard_widget(uid: int) -> dict[str, str | int]:
    """Dashboard 聚合：参数类型写成 Awaitable，便于单测注入 Future。"""
    tier_aw: Awaitable[str] = fetch_user_tier(uid)
    quota_aw: Awaitable[int] = fetch_quota_headroom(uid)
    tier, quota = await asyncio.gather(tier_aw, quota_aw)
    return {"tier": tier, "quota": quota}


def typing_examples_main() -> None:
    print("=== 11) typing：贴近真实工程的一小段「竖切」演示 ===\n")

    print("【A】网关 / 入口：Literal 限定监听端口，避免魔法数")
    print(" ", bind_socket_hint(443))

    print("\n【B】可观测性：Literal 日志级别 → 结构化一行（可送 ELK）")
    print(" ", emit_structured("INFO", "checkout", "payment captured"))

    print("\n【C】埋点 / 计费窗口：TypeVar 约束「起止时间同一类型」")
    print("  epoch 窗口:", merge_same_type_window(1_700_000_000, 1_700_000_060))
    print("  ISO 窗口: ", merge_same_type_window("2024-01-01T00:00:00Z", "2024-01-01T00:05:00Z"))
    print("  （若写成 merge_same_type_window(1, 'x')，mypy/pyright 会报类型错误；运行时 Python 不拦。）")

    print("\n【D】日终对账：Protocol 统一「能导出 CSV 行」的供应商适配器")
    append_to_daily_export(LegacyErpAdapter(90012))
    append_to_daily_export(StripeLikeAdapter("ch_3abc"))
    print("  isinstance(StripeLikeAdapter('x'), InvoiceExporter) =", isinstance(StripeLikeAdapter("x"), InvoiceExporter))

    print("\n【E】用户服务：Annotated UserId + 自建校验 + 运行时拆元数据")
    uid = validate_user_id(501)
    print(" ", describe_user(uid))
    explain_annotated_schema()

    print("\n【F】BFF / Dashboard：Awaitable 聚合多个下游协程")
    print(" ", asyncio.run(dashboard_widget(501)))


if __name__ == "__main__":
    typing_examples_main()

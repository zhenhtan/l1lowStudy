"""Python 面试重点 08: 内存与拷贝（浅拷贝/深拷贝 + 可变对象陷阱）"""

import copy


def copy_demo() -> None:
    origin = {
        "name": "alice",
        "scores": [90, 95],
        "meta": {"city": "shanghai"},
    }

    shallow = copy.copy(origin)  # 只拷贝第一层容器
    deep = copy.deepcopy(origin)  # 递归拷贝所有层级

    # 修改浅拷贝中的嵌套对象，会影响 origin
    shallow["scores"].append(100)
    shallow["meta"]["city"] = "beijing"

    # 修改深拷贝中的嵌套对象，不影响 origin
    deep["scores"].append(60)
    deep["meta"]["city"] = "shenzhen"

    print("origin:", origin)
    print("shallow:", shallow)
    print("deep:", deep)


def mutable_default_arg_trap(x: int, buf: list[int] = []) -> list[int]:
    # 面试高频陷阱: 默认参数只在函数定义时创建一次
    buf.append(x)
    return buf


def safe_version(x: int, buf: list[int] | None = None) -> list[int]:
    if buf is None:
        buf = []
    buf.append(x)
    return buf


def memory_demo() -> None:
    """用贴近业务的故事串起：浅/深拷贝、可变默认参数。"""
    print("=== 场景: 电商配置缓存 + 网关订单流水（内存语义）===\n")

    # --- 浅拷贝误伤「全局商品模板」---
    print("--- 1) 运营从内存里的「商品模板」派生活动页（浅拷贝）---")
    catalog_template: dict = {
        "sku": "SNK-001",
        "tags": ["热销", "零食"],
        "stock_by_city": {"上海": 100, "北京": 50},
    }
    flash_sale = copy.copy(catalog_template)
    flash_sale["tags"].append("618专享")
    flash_sale["stock_by_city"]["上海"] = 80  # 只想改活动库存

    print("活动页只改了 flash_sale，但嵌套 list/dict 与模板共享引用 → 线上主数据被污染:")
    print("  主模板 catalog_template:", catalog_template)
    print("  活动页 flash_sale:      ", flash_sale)

    # --- 深拷贝：每个活动独立 ---
    print("\n--- 2) 用深拷贝为「另一档活动」做隔离副本 ---")
    catalog_template = {
        "sku": "SNK-001",
        "tags": ["热销", "零食"],
        "stock_by_city": {"上海": 100, "北京": 50},
    }
    vip_day = copy.deepcopy(catalog_template)
    vip_day["tags"].append("会员日")
    vip_day["stock_by_city"]["上海"] = 60

    print("改 vip_day 后，主模板保持独立（适合 Redis 拉模板再 deepcopy 到本地会话）:")
    print("  主模板 catalog_template:", catalog_template)
    print("  活动页 vip_day:          ", vip_day)

    # --- 可变默认参数：网关按请求累积「最近订单号」---
    print("\n--- 3) 网关中间件把最近下单 ID 记在列表里（错误 vs 正确默认参数）---")
    print("错误写法: def append_recent(order_id, recent=[]) → 同一进程里请求共享一个 list")
    print("  用户甲下单 90001 →", mutable_default_arg_trap(90001))
    print("  用户乙下单 70002 →", mutable_default_arg_trap(70002), "（乙「继承」了甲的 ID，串单）")

    print("\n正确写法: recent=None，函数内再 recent = []")
    print("  用户甲下单 90001 →", safe_version(90001))
    print("  用户乙下单 70002 →", safe_version(70002))

    print("\n（附录）最小数据上的 copy 对比，可与上面 1) 对照阅读:")
    copy_demo()


if __name__ == "__main__":
    memory_demo()

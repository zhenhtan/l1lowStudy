"""Python 面试补充 10: 一个普通 class 里常见「成员」有哪些

说明顺序：类体里能写什么 → 实例上能看到什么 → 和 __dict__ / 描述符的关系。
"""

from __future__ import annotations


def class_members_demo() -> None:
    # 类定义在函数内，避免模块 import 时污染全局命名空间；语义与普通顶层类相同。
    class Device:
        """文档串：存在类对象 Device.__doc__，帮助 IDE / help()。"""

        category = "IoT"  # 类属性：所有实例共享（除非实例同名遮蔽）

        def __init__(self, device_id: str) -> None:
            # 构造里绑到 self 上的 → 实例属性（每个实例一份）
            self.device_id = device_id
            self._serial = device_id.upper()  # 单下划线：约定「内部用」，仍可从外访问

        # --- 实例方法：第一个参数 self，通过实例调用时自动传入 ---
        def ping(self) -> str:
            return f"pong:{self.device_id}"

        # --- 类方法：第一个参数 cls，可用来写「命名构造器」---
        @classmethod
        def from_mac(cls, mac: str) -> Device:
            #print(f"from_mac: cls {cls}, mac {mac}")
            normalized = mac.replace(":", "")
            return cls(f"mac-{normalized}")

        # --- 静态方法：逻辑上属于类命名空间，但不收 self/cls ---
        @staticmethod
        def validate_id(device_id: str) -> bool:
            return len(device_id) >= 3

        # --- property：访问时像属性，底层是描述符协议 ---
        @property
        def label(self) -> str:
            return f"[{self.category}] {self.device_id}"

        # --- 常用魔术方法：控制 repr/str/相等 等 ---
        def __repr__(self) -> str: # 打印对象时会调用这个方法
            return f"Device({self.device_id!r})"

    print("=== 10) 普通 Python class 常见成员 ===\n")

    d = Device("sensor-01")
    d2 = Device.from_mac("aa:bb:cc:dd:ee:ff")

    print("1) 类属性（定义在类体、未绑在 self 上）")
    print("   Device.category =", repr(Device.category))
    print("   d.category 先找实例再找类 →", repr(d.category))

    print("\n2) 实例属性（__init__ 里 self.xxx = …）")
    print("   d.__dict__ =", d.__dict__)

    print("\n3) 方法种类")
    print("   实例方法:", d.ping())
    print("   类方法:  ", d2, "→", d2.ping())
    print("   静态方法:", Device.validate_id("ab"), Device.validate_id("abc"))

    print("\n4) @property → 像读属性一样调用方法")
    print("   d.label =", d.label)

    print("\n5) 类对象的命名空间（节选，含方法、descriptor 等）")
    interesting = ("category", "__init__", "ping", "from_mac", "validate_id", "label", "__repr__")
    for name in interesting:
        if name in Device.__dict__:
            print(f"   Device.__dict__[{name!r}] → {type(Device.__dict__[name]).__name__}")

    print("\n6) 小结（面试可背）")
    print("   - 类属性: 类体顶层赋值，多实例共享。")
    print("   - 实例属性: 通常 __init__ 里 self.x = …，存在实例 __dict__（未用 __slots__ 时）。")
    print("   - 实例方法 / 类方法 / 静态方法: 都是类上挂的函数对象，通过描述符绑定 self 或 cls。")
    print("   - property: 描述符，控制读/写/删属性逻辑。")
    print("   - 魔术方法: __repr__、__eq__、__len__ 等，参与语言语法或内置函数行为。")
    print("   - 文档: __doc__；模块里还有 __name__、__qualname__、__module__ 等元数据。")


if __name__ == "__main__":
    class_members_demo()

"""反转单链表示例（含关键地址打印）"""

from __future__ import annotations

from dataclasses import dataclass
from typing import Iterable, Optional


@dataclass
class ListNode:
    val: int
    next: Optional["ListNode"] = None


def addr(node: Optional[ListNode]) -> str:
    """返回节点地址，None 时返回空指针样式。"""
    return "0x0" if node is None else hex(id(node))


def build_linked_list(values: Iterable[int]) -> Optional[ListNode]:
    head: Optional[ListNode] = None
    tail: Optional[ListNode] = None

    for v in values:
        node = ListNode(v)
        if head is None:
            head = node
            tail = node
        else:
            assert tail is not None
            tail.next = node
            tail = node

    return head


def print_chain(head: Optional[ListNode], title: str) -> None:
    print(f"\n{title}")
    cur = head
    idx = 0
    while cur is not None:
        print(
            f"  idx={idx}, val={cur.val}, node={addr(cur)}, next={addr(cur.next)}"
        )
        cur = cur.next
        idx += 1


def reverse_linked_list(head: Optional[ListNode]) -> Optional[ListNode]:
    prev: Optional[ListNode] = None
    cur = head
    step = 0

    print("\n开始反转：")
    while cur is not None:
        nxt = cur.next
        print(
            f"  step={step}, cur={addr(cur)}, prev={addr(prev)}, next(before)={addr(nxt)}"
        )

        # 核心反转：当前节点指向前驱
        cur.next = prev
        print(
            f"  step={step}, 修改后: node {addr(cur)} 的 next -> {addr(cur.next)}"
        )

        prev = cur
        cur = nxt
        step += 1

    print(f"反转结束：new_head={addr(prev)}")
    return prev


def main() -> None:
    values = [1, 2, 3, 4, 5]
    head = build_linked_list(values)

    print_chain(head, "原链表")
    new_head = reverse_linked_list(head)
    print_chain(new_head, "反转后链表")


if __name__ == "__main__":
    main()

#include <stdio.h>
#include <stdlib.h>

#include "rbtree.h"

/* 中序遍历回调：打印 key 和颜色 */
static void print_node(int key, RBColor color, void *ctx)
{
    (void)ctx;
    printf("  %d(%s)", key, color == RB_RED ? "R" : "B");
}

static void section(const char *title)
{
    printf("\n══════════ %s ══════════\n", title);
}

int main(void)
{
    RBTree *t = rbtree_create();
    if (t == NULL) {
        fputs("OOM\n", stderr);
        return 1;
    }

    /* ── 1. 插入 ─────────────────────────────────────── */
    section("插入 10 个节点");

    const int keys[] = {41, 38, 31, 12, 19, 8, 50, 35, 24, 7};
    const int n      = (int)(sizeof(keys) / sizeof(keys[0]));

    for (int i = 0; i < n; ++i) {
        int ret = rbtree_insert(t, keys[i]);
        printf("  insert(%2d) → %s, size=%zu\n",
               keys[i], ret == 0 ? "OK" : "FAIL(dup)", t->size);
    }

    /* 重复插入测试 */
    printf("  insert(%2d) → %s  (预期 FAIL)\n",
           12, rbtree_insert(t, 12) == 0 ? "OK" : "FAIL(dup)");

    /* ── 2. 树形打印 ──────────────────────────────────── */
    section("树形结构（L=左孩子 R=右孩子，括号内B=黑 R=红）");
    rbtree_print(t);

    /* ── 3. 中序遍历（结果应为升序） ─────────────────── */
    section("中序遍历（升序）");
    rbtree_inorder(t, print_node, NULL);
    putchar('\n');

    /* ── 4. 验证红黑树性质 ────────────────────────────── */
    section("验证红黑树性质");
    printf("  verify → %s\n", rbtree_verify(t) ? "PASS ✓" : "FAIL ✗");

    /* ── 5. 查找 ─────────────────────────────────────── */
    section("查找");
    const int search_keys[] = {19, 50, 99};
    for (int i = 0; i < 3; ++i) {
        RBNode *found = rbtree_search(t, search_keys[i]);
        if (found != NULL) {
            printf("  search(%2d) → 找到，颜色=%s\n",
                   search_keys[i],
                   found->color == RB_RED ? "RED" : "BLACK");
        } else {
            printf("  search(%2d) → 未找到\n", search_keys[i]);
        }
    }

    /* ── 6. 删除几个节点 ─────────────────────────────── */
    section("删除 38, 19, 7");
    const int del_keys[] = {38, 19, 7};
    for (int i = 0; i < 3; ++i) {
        int ret = rbtree_delete(t, del_keys[i]);
        printf("  delete(%2d) → %s, size=%zu\n",
               del_keys[i], ret == 0 ? "OK" : "FAIL(not found)", t->size);
    }
    /* 删除不存在的 key */
    printf("  delete(%2d) → %s  (预期 FAIL)\n",
           99, rbtree_delete(t, 99) == 0 ? "OK" : "FAIL(not found)");

    /* ── 7. 删除后的树形和验证 ────────────────────────── */
    section("删除后的树形结构");
    rbtree_print(t);

    section("删除后中序遍历");
    rbtree_inorder(t, print_node, NULL);
    putchar('\n');

    section("删除后验证红黑树性质");
    printf("  verify → %s\n", rbtree_verify(t) ? "PASS ✓" : "FAIL ✗");

    /* ── 8. 清空树 ───────────────────────────────────── */
    section("清空树（逐一删除所有节点）");
    const int remaining[] = {41, 31, 12, 8, 50, 35, 24};
    for (int i = 0; i < 7; ++i) {
        rbtree_delete(t, remaining[i]);
    }
    printf("  size=%zu\n", t->size);
    rbtree_print(t);

    rbtree_destroy(t);
    puts("\n[OK] 完成");
    return 0;
}

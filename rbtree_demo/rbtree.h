#ifndef RBTREE_H
#define RBTREE_H

#include <stddef.h>

/* ── 颜色 ────────────────────────────────────────────────── */
typedef enum { RB_RED = 0, RB_BLACK = 1 } RBColor;

/* ── 节点 ────────────────────────────────────────────────── */
typedef struct RBNode {
    int            key;
    RBColor        color;
    struct RBNode *parent;
    struct RBNode *left;
    struct RBNode *right;
} RBNode;

/* ── 树（含哨兵 NIL 节点） ──────────────────────────────── */
typedef struct {
    RBNode *root;
    RBNode *nil;   /* 哨兵：所有叶子/空指针都指向它，颜色恒为 BLACK */
    size_t  size;
} RBTree;

/* ── 生命周期 ───────────────────────────────────────────── */
RBTree *rbtree_create(void);
void    rbtree_destroy(RBTree *t);

/* ── 基本操作 ───────────────────────────────────────────── */
int     rbtree_insert(RBTree *t, int key);   /* 0=成功，-1=已存在或OOM */
int     rbtree_delete(RBTree *t, int key);   /* 0=成功，-1=不存在 */
RBNode *rbtree_search(const RBTree *t, int key);

/* ── 遍历 ───────────────────────────────────────────────── */
typedef void (*RBVisitor)(int key, RBColor color, void *ctx);
void rbtree_inorder(const RBTree *t, RBVisitor fn, void *ctx);

/* ── 调试：ASCII 树形打印 ───────────────────────────────── */
void rbtree_print(const RBTree *t);

/* ── 验证红黑树性质（返回 1=合法，0=违反） ─────────────── */
int rbtree_verify(const RBTree *t);

#endif /* RBTREE_H */

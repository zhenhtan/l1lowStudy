#include "rbtree.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ════════════════════════════════════════════════════════════
 *  内部辅助宏
 * ════════════════════════════════════════════════════════════ */
#define IS_RED(t, n)   ((n) != (t)->nil && (n)->color == RB_RED)
#define IS_BLACK(t, n) (!IS_RED(t, n))

/* ════════════════════════════════════════════════════════════
 *  生命周期
 * ════════════════════════════════════════════════════════════ */
RBTree *rbtree_create(void)
{
    RBTree *t = (RBTree *)malloc(sizeof(*t));
    if (t == NULL) {
        return NULL;
    }

    /* 哨兵 NIL：颜色恒为 BLACK，其余字段无意义 */
    t->nil = (RBNode *)calloc(1U, sizeof(RBNode));
    if (t->nil == NULL) {
        free(t);
        return NULL;
    }
    t->nil->color = RB_BLACK;

    t->root = t->nil;
    t->size = 0U;
    return t;
}

/* 递归释放以 n 为根的子树（跳过哨兵） */
static void free_subtree(RBTree *t, RBNode *n)
{
    if (n == t->nil) {
        return;
    }
    free_subtree(t, n->left);
    free_subtree(t, n->right);
    free(n);
}

void rbtree_destroy(RBTree *t)
{
    if (t == NULL) {
        return;
    }
    free_subtree(t, t->root);
    free(t->nil);
    free(t);
}

/* ════════════════════════════════════════════════════════════
 *  旋转
 *
 *    左旋 rotate_left(t, x)        右旋 rotate_right(t, y)
 *
 *      x                y               y              x
 *     / \      →       / \             / \    →        / \
 *    a   y            x   c           x   c           a   y
 *       / \          / \             / \                 / \
 *      b   c        a   b           a   b               b   c
 * ════════════════════════════════════════════════════════════ */
static void rotate_left(RBTree *t, RBNode *x)
{
    RBNode *y = x->right;
    x->right  = y->left;

    if (y->left != t->nil) {
        y->left->parent = x;
    }

    y->parent = x->parent;

    if (x->parent == t->nil) {
        t->root = y;
    } else if (x == x->parent->left) {
        x->parent->left = y;
    } else {
        x->parent->right = y;
    }

    y->left   = x;
    x->parent = y;
}

static void rotate_right(RBTree *t, RBNode *y)
{
    RBNode *x = y->left;
    y->left   = x->right;

    if (x->right != t->nil) {
        x->right->parent = y;
    }

    x->parent = y->parent;

    if (y->parent == t->nil) {
        t->root = x;
    } else if (y == y->parent->left) {
        y->parent->left = x;
    } else {
        y->parent->right = x;
    }

    x->right  = y;
    y->parent = x;
}

/* ════════════════════════════════════════════════════════════
 *  插入修复
 *
 *  新节点着色为 RED，可能违反"红节点的子节点必须是黑色"规则。
 *  分三种情况（叔叔红/叔叔黑+折线/叔叔黑+直线），左右对称共六种。
 * ════════════════════════════════════════════════════════════ */
static void insert_fixup(RBTree *t, RBNode *z)
{
    while (IS_RED(t, z->parent)) {
        if (z->parent == z->parent->parent->left) {
            RBNode *uncle = z->parent->parent->right;

            if (IS_RED(t, uncle)) {
                /* Case 1：叔叔是红色 → 重新着色，问题上移 */
                z->parent->color          = RB_BLACK;
                uncle->color              = RB_BLACK;
                z->parent->parent->color  = RB_RED;
                z = z->parent->parent;
            } else {
                if (z == z->parent->right) {
                    /* Case 2：叔叔黑，z 是右孩子（折线）→ 左旋转成直线 */
                    z = z->parent;
                    rotate_left(t, z);
                }
                /* Case 3：叔叔黑，z 是左孩子（直线）→ 右旋 + 重新着色 */
                z->parent->color         = RB_BLACK;
                z->parent->parent->color = RB_RED;
                rotate_right(t, z->parent->parent);
            }
        } else {
            /* 镜像：父节点是祖父节点的右孩子 */
            RBNode *uncle = z->parent->parent->left;

            if (IS_RED(t, uncle)) {
                z->parent->color          = RB_BLACK;
                uncle->color              = RB_BLACK;
                z->parent->parent->color  = RB_RED;
                z = z->parent->parent;
            } else {
                if (z == z->parent->left) {
                    z = z->parent;
                    rotate_right(t, z);
                }
                z->parent->color         = RB_BLACK;
                z->parent->parent->color = RB_RED;
                rotate_left(t, z->parent->parent);
            }
        }
    }
    t->root->color = RB_BLACK;   /* 性质 2：根永远是黑色 */
}

int rbtree_insert(RBTree *t, int key)
{
    RBNode *y = t->nil;
    RBNode *x = t->root;

    /* 找到插入位置 */
    while (x != t->nil) {
        y = x;
        if (key < x->key) {
            x = x->left;
        } else if (key > x->key) {
            x = x->right;
        } else {
            return -1;   /* 已存在，不允许重复 key */
        }
    }

    RBNode *z = (RBNode *)malloc(sizeof(*z));
    if (z == NULL) {
        return -1;
    }
    z->key    = key;
    z->color  = RB_RED;   /* 新节点先着红色 */
    z->left   = t->nil;
    z->right  = t->nil;
    z->parent = y;

    if (y == t->nil) {
        t->root = z;
    } else if (key < y->key) {
        y->left = z;
    } else {
        y->right = z;
    }

    ++t->size;
    insert_fixup(t, z);
    return 0;
}

/* ════════════════════════════════════════════════════════════
 *  删除
 *
 *  步骤：
 *  1. 找到节点 z
 *  2. 确定实际被移走的节点 y（z 只有一个孩子或没孩子，y=z；
 *     否则 y = z 的中序后继）
 *  3. 用 y 的孩子 x 替换 y 的位置（transplant）
 *  4. 若 y 原本是黑色，调用 delete_fixup 修复双黑问题
 * ════════════════════════════════════════════════════════════ */

/* 用以 v 为根的子树替换以 u 为根的子树 */
static void transplant(RBTree *t, RBNode *u, RBNode *v)
{
    if (u->parent == t->nil) {
        t->root = v;
    } else if (u == u->parent->left) {
        u->parent->left = v;
    } else {
        u->parent->right = v;
    }
    v->parent = u->parent;   /* 允许 v == t->nil，哨兵的 parent 会被修改 */
}

/* 子树最小节点 */
static RBNode *subtree_min(RBTree *t, RBNode *n)
{
    while (n->left != t->nil) {
        n = n->left;
    }
    return n;
}

static void delete_fixup(RBTree *t, RBNode *x)
{
    while (x != t->root && IS_BLACK(t, x)) {
        if (x == x->parent->left) {
            RBNode *w = x->parent->right;   /* 兄弟节点 */

            if (IS_RED(t, w)) {
                /* Case 1：兄弟是红色 → 转成黑色兄弟的情形 */
                w->color         = RB_BLACK;
                x->parent->color = RB_RED;
                rotate_left(t, x->parent);
                w = x->parent->right;
            }

            if (IS_BLACK(t, w->left) && IS_BLACK(t, w->right)) {
                /* Case 2：兄弟两孩子都是黑色 → 兄弟变红，问题上移 */
                w->color = RB_RED;
                x = x->parent;
            } else {
                if (IS_BLACK(t, w->right)) {
                    /* Case 3：兄弟右孩子黑，左孩子红（折线）→ 右旋转成直线 */
                    w->left->color = RB_BLACK;
                    w->color       = RB_RED;
                    rotate_right(t, w);
                    w = x->parent->right;
                }
                /* Case 4：兄弟右孩子是红色（直线）→ 旋转 + 重新着色，结束 */
                w->color         = x->parent->color;
                x->parent->color = RB_BLACK;
                w->right->color  = RB_BLACK;
                rotate_left(t, x->parent);
                x = t->root;
            }
        } else {
            /* 镜像 */
            RBNode *w = x->parent->left;

            if (IS_RED(t, w)) {
                w->color         = RB_BLACK;
                x->parent->color = RB_RED;
                rotate_right(t, x->parent);
                w = x->parent->left;
            }

            if (IS_BLACK(t, w->right) && IS_BLACK(t, w->left)) {
                w->color = RB_RED;
                x = x->parent;
            } else {
                if (IS_BLACK(t, w->left)) {
                    w->right->color = RB_BLACK;
                    w->color        = RB_RED;
                    rotate_left(t, w);
                    w = x->parent->left;
                }
                w->color         = x->parent->color;
                x->parent->color = RB_BLACK;
                w->left->color   = RB_BLACK;
                rotate_right(t, x->parent);
                x = t->root;
            }
        }
    }
    x->color = RB_BLACK;
}

int rbtree_delete(RBTree *t, int key)
{
    /* 查找目标节点 */
    RBNode *z = t->root;
    while (z != t->nil) {
        if (key < z->key) {
            z = z->left;
        } else if (key > z->key) {
            z = z->right;
        } else {
            break;
        }
    }
    if (z == t->nil) {
        return -1;   /* 不存在 */
    }

    RBNode *y         = z;
    RBColor y_orig    = y->color;
    RBNode *x         = t->nil;

    if (z->left == t->nil) {
        x = z->right;
        transplant(t, z, z->right);
    } else if (z->right == t->nil) {
        x = z->left;
        transplant(t, z, z->left);
    } else {
        /* z 有两个孩子：找中序后继 y（右子树最小） */
        y        = subtree_min(t, z->right);
        y_orig   = y->color;
        x        = y->right;

        if (y->parent == z) {
            x->parent = y;   /* 若 x 是哨兵，也要设好 parent */
        } else {
            transplant(t, y, y->right);
            y->right         = z->right;
            y->right->parent = y;
        }

        transplant(t, z, y);
        y->left         = z->left;
        y->left->parent = y;
        y->color        = z->color;
    }

    free(z);
    --t->size;

    if (y_orig == RB_BLACK) {
        delete_fixup(t, x);
    }
    return 0;
}

/* ════════════════════════════════════════════════════════════
 *  查找
 * ════════════════════════════════════════════════════════════ */
RBNode *rbtree_search(const RBTree *t, int key)
{
    RBNode *n = t->root;
    while (n != t->nil) {
        if (key < n->key) {
            n = n->left;
        } else if (key > n->key) {
            n = n->right;
        } else {
            return n;
        }
    }
    return NULL;
}

/* ════════════════════════════════════════════════════════════
 *  中序遍历（递归）
 * ════════════════════════════════════════════════════════════ */
static void inorder_rec(const RBTree *t, RBNode *n, RBVisitor fn, void *ctx)
{
    if (n == t->nil) {
        return;
    }
    inorder_rec(t, n->left,  fn, ctx);
    fn(n->key, n->color, ctx);
    inorder_rec(t, n->right, fn, ctx);
}

void rbtree_inorder(const RBTree *t, RBVisitor fn, void *ctx)
{
    inorder_rec(t, t->root, fn, ctx);
}

/* ════════════════════════════════════════════════════════════
 *  ASCII 树形打印
 *
 *  使用前缀字符串递归打印：
 *      R----12(R)
 *           L----5(B)
 *           |    L----3(R)
 *           |    R----8(R)
 *           R----18(B)
 * ════════════════════════════════════════════════════════════ */
#define PRINT_BUF 256

static void print_rec(const RBTree *t, RBNode *n,
                      char *prefix, int is_left)
{
    if (n == t->nil) {
        return;
    }

    printf("%s", prefix);
    printf("%s", is_left ? "L----" : "R----");
    printf("%d(%s)\n", n->key, n->color == RB_RED ? "R" : "B");

    char new_prefix[PRINT_BUF];
    (void)snprintf(new_prefix, sizeof(new_prefix), "%s%s",
                   prefix, is_left ? "|    " : "     ");

    print_rec(t, n->left,  new_prefix, 1);
    print_rec(t, n->right, new_prefix, 0);
}

void rbtree_print(const RBTree *t)
{
    if (t->root == t->nil) {
        puts("(empty tree)");
        return;
    }
    printf("root: %d(%s)\n",
           t->root->key,
           t->root->color == RB_RED ? "R" : "B");
    print_rec(t, t->root->left,  "", 1);
    print_rec(t, t->root->right, "", 0);
}

/* ════════════════════════════════════════════════════════════
 *  验证红黑树五条性质
 *
 *  返回 1 = 合法，0 = 违反
 * ════════════════════════════════════════════════════════════ */
/* 返回以 n 为根的子树的黑高度，-1 表示违反 */
static int check_rec(const RBTree *t, RBNode *n)
{
    if (n == t->nil) {
        return 0;   /* 叶子（NIL）黑高度为 0 */
    }

    /* 性质 4：红节点的孩子必须是黑色 */
    if (IS_RED(t, n)) {
        if (IS_RED(t, n->left) || IS_RED(t, n->right)) {
            fprintf(stderr, "[VERIFY] 性质4违反：节点 %d 是红色但有红孩子\n", n->key);
            return -1;
        }
    }

    int lh = check_rec(t, n->left);
    int rh = check_rec(t, n->right);

    if ((lh == -1) || (rh == -1)) {
        return -1;
    }

    /* 性质 5：左右子树黑高度必须相等 */
    if (lh != rh) {
        fprintf(stderr, "[VERIFY] 性质5违反：节点 %d 左黑高=%d 右黑高=%d\n",
                n->key, lh, rh);
        return -1;
    }

    return lh + (IS_BLACK(t, n) ? 1 : 0);
}

int rbtree_verify(const RBTree *t)
{
    /* 性质 2：根是黑色 */
    if (IS_RED(t, t->root)) {
        fprintf(stderr, "[VERIFY] 性质2违反：根节点是红色\n");
        return 0;
    }
    return check_rec(t, t->root) != -1 ? 1 : 0;
}

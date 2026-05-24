/* 二叉树前序遍历示例（递归和迭代） */

#include <stdio.h>
#include <stdlib.h>

struct TreeNode {
    int val;
    struct TreeNode *left;
    struct TreeNode *right;
};

static struct TreeNode *create_node(int value)
{
    struct TreeNode *node = (struct TreeNode *)malloc(sizeof(struct TreeNode));
    if (node == NULL) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }
    node->val = value;
    node->left = NULL;
    node->right = NULL;
    return node;
}

/* 前序遍历递归实现：根 -> 左 -> 右 */
static void preorder_recursive(struct TreeNode *root, int *result, int *index)
{
    if (root == NULL) {
        return;
    }
    result[(*index)] = root->val;  /* 访问根节点 */
    (*index)++; /* 更新结果数组的索引 */
    printf("访问节点: %d, index=%d\n", root->val, *index - 1);  /* 打印访问的节点值 */
    preorder_recursive(root->left, result, index);    /* 遍历左子树 */
    preorder_recursive(root->right, result, index);   /* 遍历右子树 */
}
int *preorder_traversal_recursive(struct TreeNode *root, int *return_size)
{
    int *result = (int *)malloc(100 * sizeof(int));
    if (result == NULL) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }
    int index = 0;
    preorder_recursive(root, result, &index);
    *return_size = index;
    
    return result;
}

/* 栈结构用于迭代实现 */
typedef struct {
    struct TreeNode *nodes[100];
    int top;
} Stack;

static void stack_push(Stack *stack, struct TreeNode *node)
{
    if (stack->top < 100) {
        stack->nodes[stack->top++] = node;
    }
}

static struct TreeNode *stack_pop(Stack *stack)
{
    if (stack->top > 0) {
        return stack->nodes[--stack->top];
    }
    return NULL;
}

static int stack_is_empty(Stack *stack)
{
    return stack->top == 0;
}

/* 前序遍历迭代实现 */
int *preorder_traversal_iterative(struct TreeNode *root, int *return_size)
{
    int *result = (int *)malloc(100 * sizeof(int));
    if (result == NULL) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }
    
    if (root == NULL) {
        *return_size = 0;
        return result;
    }
    
    Stack stack = {0};
    stack_push(&stack, root);
    int index = 0;
    
    while (!stack_is_empty(&stack)) {
        struct TreeNode *node = stack_pop(&stack);
        result[index++] = node->val;  /* 访问当前节点 */
        
        /* 注意：先压右子树再压左子树，因为栈是后进先出 */
        if (node->right != NULL) {
            stack_push(&stack, node->right);
        }
        if (node->left != NULL) {
            stack_push(&stack, node->left);
        }
    }
    
    *return_size = index;
    return result;
}

/* 后序遍历递归实现：左 -> 右 -> 根 */
static void postorder_recursive(struct TreeNode *root, int *result, int *index)
{
    if (root == NULL) {
        return;
    }
    
    postorder_recursive(root->left, result, index);    /* 遍历左子树 */
    postorder_recursive(root->right, result, index);   /* 遍历右子树 */
    result[(*index)++] = root->val;  /* 访问根节点 */
    printf("访问节点: %d, index=%d\n", root->val, *index - 1);  /* 打印访问的节点值 */
}

int *postorder_traversal_recursive(struct TreeNode *root, int *return_size)
{
    int *result = (int *)malloc(100 * sizeof(int));
    if (result == NULL) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }
    
    int index = 0;
    postorder_recursive(root, result, &index);
    *return_size = index;
    
    return result;
}

/* 后序遍历迭代实现（使用两个栈或标记法） */
int *postorder_traversal_iterative(struct TreeNode *root, int *return_size)
{
    int *result = (int *)malloc(100 * sizeof(int));
    if (result == NULL) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }
    
    if (root == NULL) {
        *return_size = 0;
        return result;
    }
    
    /* 使用两个栈实现后序遍历 */
    Stack stack1 = {0}, stack2 = {0};
    stack_push(&stack1, root);
    
    while (!stack_is_empty(&stack1)) {
        struct TreeNode *node = stack_pop(&stack1);
        stack_push(&stack2, node);
        
        if (node->left != NULL) {
            stack_push(&stack1, node->left);
        }
        if (node->right != NULL) {
            stack_push(&stack1, node->right);
        }
    }
    
    int index = 0;
    while (!stack_is_empty(&stack2)) {
        struct TreeNode *node = stack_pop(&stack2);
        result[index++] = node->val;
    }
    
    *return_size = index;
    return result;
}

/* 中序遍历递归实现：左 -> 根 -> 右 */
static void inorder_recursive(struct TreeNode *root, int *result, int *index)
{
    if (root == NULL) {
        return;
    }
    
    inorder_recursive(root->left, result, index);     /* 遍历左子树 */
    result[(*index)++] = root->val;  /* 访问根节点 */
    printf("访问节点: %d, index=%d\n", root->val, *index - 1);  /* 打印访问的节点值 */
    inorder_recursive(root->right, result, index);    /* 遍历右子树 */
}

int *inorder_traversal_recursive(struct TreeNode *root, int *return_size)
{
    int *result = (int *)malloc(100 * sizeof(int));
    if (result == NULL) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }
    
    int index = 0;
    inorder_recursive(root, result, &index);
    *return_size = index;
    
    return result;
}

/* 中序遍历迭代实现（使用栈） */
int *inorder_traversal_iterative(struct TreeNode *root, int *return_size)
{
    int *result = (int *)malloc(100 * sizeof(int));
    if (result == NULL) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }
    
    if (root == NULL) {
        *return_size = 0;
        return result;
    }
    
    Stack stack = {0};
    struct TreeNode *current = root;
    int index = 0;
    
    while (current != NULL || !stack_is_empty(&stack)) {
        /* 遍历到最左边的节点 */
        while (current != NULL) {
            stack_push(&stack, current);
            current = current->left;
        }
        
        /* 出栈并访问节点 */
        current = stack_pop(&stack);
        result[index++] = current->val;
        
        /* 访问右子树 */
        current = current->right;
    }
    
    *return_size = index;
    return result;
}

static void print_array(int *arr, int size, const char *title)
{
    printf("\n%s: [", title);
    for (int i = 0; i < size; ++i) {
        printf("%d", arr[i]);
        if (i < size - 1) {
            printf(", ");
        }
    }
    printf("]\n");
}

static void free_tree(struct TreeNode *root)
{
    if (root == NULL) {
        return;
    }
    free_tree(root->left);
    free_tree(root->right);
    free(root);
}

int main(void)
{
    /*
     * 构造二叉树：
     *       1
     *      / \
     *     2   3
     *    / \
     *   4   5
     */
    struct TreeNode *root = create_node(1);
    root->left = create_node(2);
    root->right = create_node(3);
    root->left->left = create_node(4);
    root->left->right = create_node(5);
    
    printf("二叉树结构：\n");
    printf("       1\n");
    printf("      / \\\n");
    printf("     2   3\n");
    printf("    / \\\n");
    printf("   4   5\n");
    
    int size = 0;
    int *result_recursive = preorder_traversal_recursive(root, &size);
    print_array(result_recursive, size, "前序遍历（递归）");
    
    int size_iterative = 0;
    int *result_iterative = preorder_traversal_iterative(root, &size_iterative);
    print_array(result_iterative, size_iterative, "前序遍历（迭代）");
    
    int inorder_size = 0;
    int *result_inorder_recursive = inorder_traversal_recursive(root, &inorder_size);
    print_array(result_inorder_recursive, inorder_size, "中序遍历（递归）");
    
    int inorder_iterative_size = 0;
    int *result_inorder_iterative = inorder_traversal_iterative(root, &inorder_iterative_size);
    print_array(result_inorder_iterative, inorder_iterative_size, "中序遍历（迭代）");
    
    int postorder_size = 0;
    int *result_postorder_recursive = postorder_traversal_recursive(root, &postorder_size);
    print_array(result_postorder_recursive, postorder_size, "后序遍历（递归）");
    
    int postorder_iterative_size = 0;
    int *result_postorder_iterative = postorder_traversal_iterative(root, &postorder_iterative_size);
    print_array(result_postorder_iterative, postorder_iterative_size, "后序遍历（迭代）");
    
    free(result_recursive);
    free(result_iterative);
    free(result_inorder_recursive);
    free(result_inorder_iterative);
    free(result_postorder_recursive);
    free(result_postorder_iterative);
    free_tree(root);
    
    return 0;
}

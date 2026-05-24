/* 用两个栈实现队列 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/* 栈结构 */
typedef struct {
    int *data;
    int top;
    int capacity;
} Stack;

/* 创建栈 */
static Stack *stack_create(int capacity)
{
    Stack *stack = (Stack *)malloc(sizeof(Stack));
    if (stack == NULL) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }
    stack->data = (int *)malloc(capacity * sizeof(int));
    if (stack->data == NULL) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }
    stack->top = -1;
    stack->capacity = capacity;
    return stack;
}

/* 栈压入 */
static void stack_push(Stack *stack, int value)
{
    if (stack->top + 1 < stack->capacity) {
        stack->data[++stack->top] = value;
    } else {
        printf("Stack overflow\n");
    }
}

/* 栈弹出 */
static int stack_pop(Stack *stack)
{
    if (stack->top >= 0) {
        return stack->data[stack->top--];
    }
    printf("Stack underflow\n");
    return -1;
}

/* 栈是否为空 */
static bool stack_is_empty(Stack *stack)
{
    return stack->top == -1;
}

/* 获取栈顶元素 */
static int stack_peek(Stack *stack)
{
    if (stack->top >= 0) {
        return stack->data[stack->top];
    }
    printf("Stack is empty\n");
    return -1;
}

/* 释放栈 */
static void stack_free(Stack *stack)
{
    if (stack != NULL) {
        free(stack->data);
        free(stack);
    }
}

/* 队列结构（用两个栈实现） */
typedef struct {
    Stack *stack1;  /* 用于入队 */
    Stack *stack2;  /* 用于出队 */
} QueueWithStacks;

/* 创建队列 */
static QueueWithStacks *queue_create(int capacity)
{
    QueueWithStacks *queue = (QueueWithStacks *)malloc(sizeof(QueueWithStacks));
    if (queue == NULL) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }
    queue->stack1 = stack_create(capacity);
    queue->stack2 = stack_create(capacity);
    return queue;
}
/* 打印队列（从头到尾） */
static void queue_print(QueueWithStacks *queue)
{
    printf("队列内容（从头到尾）: stack2 [");
    
    /* 创建临时栈来存储stack2的元素用于打印 */
    if (queue->stack2->top >= 0) {
        for (int i = queue->stack2->top; i >= 0; --i) {
            printf("%d", queue->stack2->data[i]);
            if (i > 0 || queue->stack1->top >= 0) {
                printf(", ");
            }
        }
    }
    printf("] ,stack1 [");
    /* 打印stack1的元素（它们在stack2之后） */
    if (queue->stack1->top >= 0) {
        for (int i = 0; i <= queue->stack1->top; ++i) {
            printf("%d", queue->stack1->data[i]);
            if (i < queue->stack1->top) {
                printf(", ");
            }
        }
    }
    
    printf("]\n");
}
/* 入队 */
static void queue_push(QueueWithStacks *queue, int value)
{
    stack_push(queue->stack1, value);
    printf("入队: %d\n", value);
}

/* 出队 */
static int queue_pop(QueueWithStacks *queue)
{
    if (stack_is_empty(queue->stack1) && stack_is_empty(queue->stack2)) {
        printf("队列为空，无法出队\n");
        return -1;
    }
    
    /* 如果stack2为空，则将stack1的所有元素转移到stack2 */
    if (stack_is_empty(queue->stack2)) {
        while (!stack_is_empty(queue->stack1)) {
            int value = stack_pop(queue->stack1);
            printf("将元素从stack1转移到stack2: %d\n", value);
            stack_push(queue->stack2, value);
            printf("当前队列状态: ");
            queue_print(queue);
        }
    }
    
    int value = stack_pop(queue->stack2);
    printf("出队: %d\n", value);
    queue_print(queue);
    return value;
}

/* 获取队列头元素（不移除） */
static int queue_peek(QueueWithStacks *queue)
{
    if (stack_is_empty(queue->stack1) && stack_is_empty(queue->stack2)) {
        printf("队列为空\n");
        return -1;
    }
    
    /* 如果stack2为空，则将stack1的所有元素转移到stack2 */
    if (stack_is_empty(queue->stack2)) {
        while (!stack_is_empty(queue->stack1)) {
            int value = stack_pop(queue->stack1);
            printf("将元素从stack1转移到stack2: %d\n", value);
            stack_push(queue->stack2, value);
        }
    }
    
    return stack_peek(queue->stack2);
}

/* 队列是否为空 */
static bool queue_is_empty(QueueWithStacks *queue)
{
    return stack_is_empty(queue->stack1) && stack_is_empty(queue->stack2);
}

/* 队列大小 */
static int queue_size(QueueWithStacks *queue)
{
    return (queue->stack1->top + 1) + (queue->stack2->top + 1);
}


/* 释放队列 */
static void queue_free(QueueWithStacks *queue)
{
    if (queue != NULL) {
        stack_free(queue->stack1);
        stack_free(queue->stack2);
        free(queue);
    }
}

int main(void)
{
    printf("=== 用两个栈实现队列 ===\n\n");
    
    QueueWithStacks *queue = queue_create(100);
    
    printf("--- 入队操作 ---\n");
    queue_push(queue, 1);
    queue_push(queue, 2);
    queue_push(queue, 3);
    queue_push(queue, 4);
    queue_push(queue, 5);
    printf("\n");
    
    printf("队列大小: %d\n", queue_size(queue));
    queue_print(queue);
    printf("\n");
    
    printf("--- 出队操作 ---\n");
    queue_pop(queue);
    queue_pop(queue);
    printf("\n");
    
    printf("队列大小: %d\n", queue_size(queue));
    queue_print(queue);
    printf("\n");
    
    printf("队列头元素（不移除）: %d\n", queue_peek(queue));
    printf("\n");
    
    printf("--- 继续入队 ---\n");
    queue_push(queue, 6);
    queue_push(queue, 7);
    printf("\n");
    
    printf("队列大小: %d\n", queue_size(queue));
    queue_print(queue);
    printf("\n");
    
    printf("--- 全部出队 ---\n");
    while (!queue_is_empty(queue)) {
        queue_pop(queue);
    }
    printf("\n");
    
    printf("队列是否为空: %s\n", queue_is_empty(queue) ? "是" : "否");
    
    queue_free(queue);
    return 0;
}

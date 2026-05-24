/* 反转单链表示例（含关键地址打印） */

#include <stdio.h>
#include <stdlib.h>

struct ListNode {
    int val;
    struct ListNode *next;
};

static void *addr(const struct ListNode *node)
{
    return (void *)node;
}

static struct ListNode *create_node(int value)
{
    struct ListNode *node = (struct ListNode *)malloc(sizeof(struct ListNode));
    if (node == NULL) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }

    node->val = value;
    node->next = NULL;
    return node;
}

static struct ListNode *build_linked_list(const int *values, int size)
{
    struct ListNode *head = NULL;
    struct ListNode *tail = NULL;

    for (int i = 0; i < size; ++i) {
        struct ListNode *node = create_node(values[i]);
        if (head == NULL) {
            head = node;
            tail = node;
        } else {
            tail->next = node;
            tail = node;
        }
    }

    return head;
}

static void print_chain(const struct ListNode *head, const char *title)
{
    printf("\n%s\n", title);
    const struct ListNode *cur = head;
    int idx = 0;
    while (cur != NULL) {
        printf("  idx=%d, val=%d, node=%p, next=%p\n",
               idx, cur->val, addr(cur), addr(cur->next));
        cur = cur->next;
        ++idx;
    }
}

static struct ListNode *reverse_linked_list(struct ListNode *head, int start, int end)
{
    int leng = 0;
    struct ListNode * leng_head= head;
    if(leng_head != NULL)
    {
	    leng++;
    }
    while( leng_head -> next != NULL)
    {
	    leng ++;
	    leng_head = leng_head->next;
    }
    if (start >= end || end > leng || start < 1)
    {
        printf("input err");
	    return head;
    }
    struct ListNode* ori_head= head;
    struct ListNode* ori_tail= head;
    struct ListNode* pre_node= NULL;
    struct ListNode* cur_node= head;
    struct ListNode* next = NULL;
    int index=0;
    while(index < start-1)
    {   pre_node = cur_node;
        cur_node = cur_node->next;
	    ori_tail=pre_node;
	    index++;
    }
    pre_node = cur_node;
    cur_node = cur_node->next;
    struct ListNode* ori_end = pre_node;
    index++;
    while(index <= end-1)
    {
        next = cur_node->next;
        cur_node->next= pre_node; // 反转后next指向原来的前一个节点
        pre_node=cur_node; // pre_node向后移动
        cur_node=next;
	    ori_tail->next=pre_node;
	    index++;
    }
    if(ori_end != NULL)
    {
        ori_end->next=cur_node;
    }
    if( 1 == start ){
        return pre_node;
    }
    else{
        return ori_head;
    }
}
static void free_chain(struct ListNode *head)
{
    while (head != NULL) {
        struct ListNode *nxt = head->next;
        free(head);
        head = nxt;
    }
}

int main(void)
{
    const int values[] = {1, 2, 3, 4, 5};
    const int size = (int)(sizeof(values) / sizeof(values[0]));

    struct ListNode *head = build_linked_list(values, size);

    print_chain(head, "原链表");
    struct ListNode *new_head = reverse_linked_list(head, 1, sizeof(values) / sizeof(values[0]));
    print_chain(new_head, "反转后链表");

    free_chain(new_head);
    return 0;
}

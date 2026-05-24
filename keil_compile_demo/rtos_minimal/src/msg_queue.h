#ifndef MSG_QUEUE_H
#define MSG_QUEUE_H

#include <stdbool.h>

#define MSG_QUEUE_CAPACITY 8

typedef struct {
    int data[MSG_QUEUE_CAPACITY];
    unsigned int head;
    unsigned int tail;
    unsigned int size;
} msg_queue_t;

void msg_queue_init(msg_queue_t *q);
bool msg_queue_push(msg_queue_t *q, int value);
bool msg_queue_pop(msg_queue_t *q, int *value);
unsigned int msg_queue_size(const msg_queue_t *q);

#endif

#include "msg_queue.h"

#include <stddef.h>

void msg_queue_init(msg_queue_t *q)
{
    if (q == NULL) {
        return;
    }

    q->head = 0U;
    q->tail = 0U;
    q->size = 0U;
}

bool msg_queue_push(msg_queue_t *q, int value)
{
    if ((q == NULL) || (q->size >= MSG_QUEUE_CAPACITY)) {
        return false;
    }

    q->data[q->tail] = value;
    q->tail = (q->tail + 1U) % MSG_QUEUE_CAPACITY;
    ++q->size;
    return true;
}

bool msg_queue_pop(msg_queue_t *q, int *value)
{
    if ((q == NULL) || (value == NULL) || (q->size == 0U)) {
        return false;
    }

    *value = q->data[q->head];
    q->head = (q->head + 1U) % MSG_QUEUE_CAPACITY;
    --q->size;
    return true;
}

unsigned int msg_queue_size(const msg_queue_t *q)
{
    if (q == NULL) {
        return 0U;
    }

    return q->size;
}

#ifndef RTOS_KERNEL_H
#define RTOS_KERNEL_H

#include <stdbool.h>

typedef void (*rtos_task_fn_t)(void);

bool rtos_add_task(const char *name, rtos_task_fn_t fn, unsigned int period_ticks);
void rtos_start(void);
void rtos_tick(void);
unsigned int rtos_get_tick(void);

#endif

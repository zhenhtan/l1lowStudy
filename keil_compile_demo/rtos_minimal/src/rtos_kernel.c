#include "rtos_kernel.h"

#include <stdio.h>

#define RTOS_MAX_TASKS 6

typedef struct {
    const char *name;
    rtos_task_fn_t fn;
    unsigned int period_ticks;
    unsigned int next_run_tick;
} rtos_task_t;

static rtos_task_t g_tasks[RTOS_MAX_TASKS];
static unsigned int g_task_count = 0;
static unsigned int g_tick = 0;
static bool g_started = false;

bool rtos_add_task(const char *name, rtos_task_fn_t fn, unsigned int period_ticks)
{
    if ((name == NULL) || (fn == NULL) || (period_ticks == 0U) || (g_task_count >= RTOS_MAX_TASKS)) {
        return false;
    }

    g_tasks[g_task_count].name = name;
    g_tasks[g_task_count].fn = fn;
    g_tasks[g_task_count].period_ticks = period_ticks;
    g_tasks[g_task_count].next_run_tick = period_ticks;
    ++g_task_count;
    return true;
}

void rtos_start(void)
{
    g_started = true;
    puts("[RTOS] Scheduler started");
}

void rtos_tick(void)
{
    if (!g_started) {
        return;
    }

    ++g_tick;

    for (unsigned int i = 0; i < g_task_count; ++i) {
        if (g_tick >= g_tasks[i].next_run_tick) {
            printf("[RTOS] tick=%u run task=%s\n", g_tick, g_tasks[i].name);
            g_tasks[i].fn();
            g_tasks[i].next_run_tick += g_tasks[i].period_ticks;
        }
    }
}

unsigned int rtos_get_tick(void)
{
    return g_tick;
}

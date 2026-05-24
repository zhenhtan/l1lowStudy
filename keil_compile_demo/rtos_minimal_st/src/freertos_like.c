#include "freertos_like.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stdint.h>
#include <sys/timerfd.h>
#include <unistd.h>
#include <time.h>

#define FREERTOS_SIM_MAX_TASKS 8
#define FREERTOS_SIM_MAX_TIMERS 8

typedef struct {
    const char *name;
    TaskFunction_t entry;
    void *params;
    UBaseType_t priority;
    TickType_t nextRunTick;
    bool used;
} TaskControl_t;

struct QueueControl {
    unsigned char *buffer;
    UBaseType_t length;
    UBaseType_t itemSize;
    UBaseType_t head;
    UBaseType_t tail;
    UBaseType_t size;
};

struct TimerControl {
    const char *name;
    TickType_t period;
    BaseType_t autoReload;
    void *id;
    TimerCallbackFunction_t callback;
    TickType_t nextExpiry;
    bool active;
    bool used;
};

static TaskControl_t g_tasks[FREERTOS_SIM_MAX_TASKS];
static struct TimerControl g_timers[FREERTOS_SIM_MAX_TIMERS];
static UBaseType_t g_taskCount = 0U;
static TickType_t g_tick = 0U;
static bool g_schedulerStarted = false;
static int g_runningTask = -1;

BaseType_t xTaskCreate(TaskFunction_t pxTaskCode,
                       const char *pcName,
                       uint16_t usStackDepth,
                       void *pvParameters,
                       UBaseType_t uxPriority,
                       void *pxCreatedTask)
{
    (void)usStackDepth;
    (void)pxCreatedTask;

    if ((pxTaskCode == NULL) || (pcName == NULL) || (g_taskCount >= FREERTOS_SIM_MAX_TASKS)) {
        return pdFAIL;
    }

    g_tasks[g_taskCount].name = pcName;
    g_tasks[g_taskCount].entry = pxTaskCode;
    g_tasks[g_taskCount].params = pvParameters;
    g_tasks[g_taskCount].priority = uxPriority;
    g_tasks[g_taskCount].nextRunTick = 1U;
    g_tasks[g_taskCount].used = true;
    ++g_taskCount;
    printf("[FreeRTOS-SIM] Created task=%s with priority=%u, total tasks=%u\n", pcName, uxPriority, g_taskCount);
    return pdPASS;
}

QueueHandle_t xQueueCreate(UBaseType_t uxQueueLength, UBaseType_t uxItemSize)
{
    struct QueueControl *queue = NULL;

    if ((uxQueueLength == 0U) || (uxItemSize == 0U)) {
        return NULL;
    }

    queue = (struct QueueControl *)calloc(1U, sizeof(*queue));
    if (queue == NULL) {
        return NULL;
    }

    queue->buffer = (unsigned char *)calloc((size_t)uxQueueLength, (size_t)uxItemSize);
    if (queue->buffer == NULL) {
        free(queue);
        return NULL;
    }

    queue->length = uxQueueLength;
    queue->itemSize = uxItemSize;
    return queue;
}

BaseType_t xQueueSend(QueueHandle_t xQueue, const void *pvItemToQueue, TickType_t xTicksToWait)
{
    (void)xTicksToWait;

    if ((xQueue == NULL) || (pvItemToQueue == NULL) || (xQueue->size >= xQueue->length)) {
        printf("[FreeRTOS-SIM] tick=%u queue full, failed to send item\n", g_tick);
        return pdFAIL;
    }

    memcpy(xQueue->buffer + ((size_t)xQueue->tail * (size_t)xQueue->itemSize),
           pvItemToQueue,
           (size_t)xQueue->itemSize);
    xQueue->tail = (xQueue->tail + 1U) % xQueue->length;
    ++xQueue->size;
    return pdPASS;
}

BaseType_t xQueueReceive(QueueHandle_t xQueue, void *pvBuffer, TickType_t xTicksToWait)
{
    (void)xTicksToWait;

    if ((xQueue == NULL) || (pvBuffer == NULL) || (xQueue->size == 0U)) {
        return pdFAIL;
    }

    memcpy(pvBuffer,
           xQueue->buffer + ((size_t)xQueue->head * (size_t)xQueue->itemSize),
           (size_t)xQueue->itemSize);
    xQueue->head = (xQueue->head + 1U) % xQueue->length;
    --xQueue->size;
    return pdPASS;
}

UBaseType_t uxQueueMessagesWaiting(QueueHandle_t xQueue)
{
    if (xQueue == NULL) {
        return 0U;
    }

    return xQueue->size;
}

TimerHandle_t xTimerCreate(const char *pcTimerName,
                           TickType_t xTimerPeriodInTicks,
                           BaseType_t xAutoReload,
                           void *pvTimerID,
                           TimerCallbackFunction_t pxCallbackFunction)
{
    for (UBaseType_t i = 0U; i < FREERTOS_SIM_MAX_TIMERS; ++i) {
        if (!g_timers[i].used) {
            if ((pcTimerName == NULL) || (xTimerPeriodInTicks == 0U) || (pxCallbackFunction == NULL)) {
                return NULL;
            }

            g_timers[i].name = pcTimerName;
            g_timers[i].period = xTimerPeriodInTicks;
            g_timers[i].autoReload = xAutoReload;
            g_timers[i].id = pvTimerID;
            g_timers[i].callback = pxCallbackFunction;
            g_timers[i].nextExpiry = 0U;
            g_timers[i].active = false;
            g_timers[i].used = true;
            return &g_timers[i];
        }
    }

    return NULL;
}

BaseType_t xTimerStart(TimerHandle_t xTimer, TickType_t xTicksToWait)
{
    (void)xTicksToWait;

    if (xTimer == NULL) {
        return pdFAIL;
    }

    xTimer->nextExpiry = g_tick + xTimer->period;
    xTimer->active = true;
    return pdPASS;
}

void vTaskDelay(TickType_t xTicksToDelay)
{
    if ((g_runningTask < 0) || ((UBaseType_t)g_runningTask >= g_taskCount)) {
        return;
    }

    g_tasks[g_runningTask].nextRunTick = g_tick + xTicksToDelay;
}

void vTaskStartScheduler(void)
{
    g_schedulerStarted = true;
    puts("[FreeRTOS-SIM] Scheduler started");
}

TickType_t xTaskGetTickCount(void)
{
    return g_tick;
}

static void process_timers(void)
{
    for (UBaseType_t i = 0U; i < FREERTOS_SIM_MAX_TIMERS; ++i) {
        if (g_timers[i].used && g_timers[i].active && (g_tick >= g_timers[i].nextExpiry)) {
            printf("[FreeRTOS-SIM] tick=%u run timer=%s\n", g_tick, g_timers[i].name);
            g_timers[i].callback(g_timers[i].id);

            if (g_timers[i].autoReload == pdTRUE) {
                g_timers[i].nextExpiry += g_timers[i].period;
            } else {
                g_timers[i].active = false;
            }
        }
    }
}

static void process_tasks(void)
{
    for (UBaseType_t prio = 10U; prio > 0U; --prio) {
        for (UBaseType_t i = 0U; i < g_taskCount; ++i) {
            if (g_tasks[i].used && (g_tasks[i].priority == (prio - 1U)) &&
                (g_tick >= g_tasks[i].nextRunTick)) {
                g_runningTask = (int)i;
                printf("[FreeRTOS-SIM] tick=%u run task=%s\n", g_tick, g_tasks[i].name);
                g_tasks[i].entry(g_tasks[i].params);
                g_runningTask = -1;
            }
        }
    }
}

void vFreeRtosSimRunTicks(TickType_t totalTicks, TickType_t tickPeriodMs)
{
    if (!g_schedulerStarted) {
        puts("[FreeRTOS-SIM] Scheduler not started");
        return;
    }

    int tfd = timerfd_create(CLOCK_MONOTONIC, 0);
    if (tfd < 0) {
        perror("[FreeRTOS-SIM] timerfd_create");
        return;
    }

    struct itimerspec its;
    its.it_value.tv_sec  = (time_t)(tickPeriodMs / 1000U);
    its.it_value.tv_nsec = (long)((tickPeriodMs % 1000U) * 1000000L);
    its.it_interval      = its.it_value;

    if (timerfd_settime(tfd, 0, &its, NULL) < 0) {
        perror("[FreeRTOS-SIM] timerfd_settime");
        (void)close(tfd);
        return;
    }

    for (TickType_t i = 0U; i < totalTicks; ++i) {
        uint64_t expirations = 0U;
        if (read(tfd, &expirations, sizeof(expirations)) < 0) {
            perror("[FreeRTOS-SIM] timerfd read");
            break;
        }
        ++g_tick;
        process_timers();
        process_tasks();
    }

    (void)close(tfd);
}

void vFreeRtosSimDestroyAll(void)
{
    for (UBaseType_t i = 0U; i < g_taskCount; ++i) {
        g_tasks[i].used = false;
    }

    for (UBaseType_t i = 0U; i < FREERTOS_SIM_MAX_TIMERS; ++i) {
        g_timers[i].used = false;
        g_timers[i].active = false;
    }

    g_taskCount = 0U;
    g_tick = 0U;
    g_schedulerStarted = false;
    g_runningTask = -1;
}

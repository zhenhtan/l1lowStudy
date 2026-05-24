#include "freertos_like.h"

#include <stdbool.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stdint.h>
#include <sys/timerfd.h>
#include <unistd.h>
#include <time.h>

#define FREERTOS_SIM_MAX_TASKS 8
#define FREERTOS_SIM_MAX_TIMERS 8
#define FREERTOS_SIM_MAX_QUEUES 8

typedef struct {
    const char *name;
    TaskFunction_t entry;
    void *params;
    UBaseType_t priority;
    TickType_t nextRunTick;
    pthread_t thread;
    bool threadStarted;
    bool used;
} TaskControl_t;

struct QueueControl {
    unsigned char *buffer;
    UBaseType_t length;
    UBaseType_t itemSize;
    UBaseType_t head;
    UBaseType_t tail;
    UBaseType_t size;
    pthread_mutex_t lock;
    pthread_cond_t notEmpty;
    pthread_cond_t notFull;
    bool used;
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
static struct QueueControl *g_queues[FREERTOS_SIM_MAX_QUEUES];

static UBaseType_t g_taskCount = 0U;
static TickType_t g_tick = 0U;
static TickType_t g_tickPeriodMs = 1U;
static bool g_schedulerStarted = false;
static bool g_stopRequested = false;

static pthread_mutex_t g_schedulerLock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t g_tickCond = PTHREAD_COND_INITIALIZER;
static _Thread_local TaskControl_t *g_currentTask = NULL;

static TickType_t get_current_tick_locked(void)
{
    return g_tick;
}

static void make_timeout_at(struct timespec *ts, TickType_t ticksToWait)
{
    const uint64_t waitMs = (uint64_t)ticksToWait * (uint64_t)((g_tickPeriodMs == 0U) ? 1U : g_tickPeriodMs);
    uint64_t nsec = 0U;

    (void)clock_gettime(CLOCK_REALTIME, ts);
    ts->tv_sec += (time_t)(waitMs / 1000U);
    nsec = (uint64_t)ts->tv_nsec + ((waitMs % 1000U) * 1000000ULL);
    ts->tv_sec += (time_t)(nsec / 1000000000ULL);
    ts->tv_nsec = (long)(nsec % 1000000000ULL);
}

static void *task_thread_main(void *arg)
{
    TaskControl_t *task = (TaskControl_t *)arg;

    while (true) {
        pthread_mutex_lock(&g_schedulerLock);
        while (!g_stopRequested && (get_current_tick_locked() < task->nextRunTick)) {
            (void)pthread_cond_wait(&g_tickCond, &g_schedulerLock);
        }

        if (g_stopRequested) {
            pthread_mutex_unlock(&g_schedulerLock);
            break;
        }

        pthread_mutex_unlock(&g_schedulerLock);

        g_currentTask = task;
        printf("[FreeRTOS-SIM] tick=%u run task=%s\n", xTaskGetTickCount(), task->name);
        task->entry(task->params);
        g_currentTask = NULL;

        pthread_mutex_lock(&g_schedulerLock);
        if (!g_stopRequested && (task->nextRunTick <= get_current_tick_locked())) {
            task->nextRunTick = get_current_tick_locked() + 1U;
        }
        pthread_mutex_unlock(&g_schedulerLock);
    }

    return NULL;
}

BaseType_t xTaskCreate(TaskFunction_t pxTaskCode,
                       const char *pcName,
                       uint16_t usStackDepth,
                       void *pvParameters,
                       UBaseType_t uxPriority,
                       void *pxCreatedTask)
{
    (void)usStackDepth;
    (void)pxCreatedTask;

    pthread_mutex_lock(&g_schedulerLock);
    if ((pxTaskCode == NULL) || (pcName == NULL) || (g_taskCount >= FREERTOS_SIM_MAX_TASKS)) {
        pthread_mutex_unlock(&g_schedulerLock);
        return pdFAIL;
    }

    g_tasks[g_taskCount].name = pcName;
    g_tasks[g_taskCount].entry = pxTaskCode;
    g_tasks[g_taskCount].params = pvParameters;
    g_tasks[g_taskCount].priority = uxPriority;
    g_tasks[g_taskCount].nextRunTick = 1U;
    g_tasks[g_taskCount].threadStarted = false;
    g_tasks[g_taskCount].used = true;
    ++g_taskCount;
    pthread_mutex_unlock(&g_schedulerLock);

    printf("[FreeRTOS-SIM] Created task=%s with priority=%u, total tasks=%u\n", pcName, uxPriority, g_taskCount);
    return pdPASS;
}

QueueHandle_t xQueueCreate(UBaseType_t uxQueueLength, UBaseType_t uxItemSize)
{
    struct QueueControl *queue = NULL;
    bool queueRegistered = false;

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
    queue->used = true;

    if ((pthread_mutex_init(&queue->lock, NULL) != 0) ||
        (pthread_cond_init(&queue->notEmpty, NULL) != 0) ||
        (pthread_cond_init(&queue->notFull, NULL) != 0)) {
        free(queue->buffer);
        free(queue);
        return NULL;
    }

    pthread_mutex_lock(&g_schedulerLock);
    for (UBaseType_t i = 0U; i < FREERTOS_SIM_MAX_QUEUES; ++i) {
        if (g_queues[i] == NULL) {
            g_queues[i] = queue;
            queueRegistered = true;
            break;
        }
    }
    pthread_mutex_unlock(&g_schedulerLock);

    if (!queueRegistered) {
        (void)pthread_cond_destroy(&queue->notFull);
        (void)pthread_cond_destroy(&queue->notEmpty);
        (void)pthread_mutex_destroy(&queue->lock);
        free(queue->buffer);
        free(queue);
        return NULL;
    }

    return queue;
}

BaseType_t xQueueSend(QueueHandle_t xQueue, const void *pvItemToQueue, TickType_t xTicksToWait)
{
    struct timespec timeoutTs;
    bool hasTimeout = false;
    bool waitLogged = false;

    if ((xQueue == NULL) || (pvItemToQueue == NULL)) {
        return pdFAIL;
    }

    pthread_mutex_lock(&xQueue->lock);

    if (xTicksToWait > 0U) {
        pthread_mutex_lock(&g_schedulerLock);
        make_timeout_at(&timeoutTs, xTicksToWait);
        pthread_mutex_unlock(&g_schedulerLock);
        hasTimeout = true;
    }

    while ((xQueue->size >= xQueue->length) && !g_stopRequested) {
        if (!hasTimeout) {
            printf("[FreeRTOS-SIM] tick=%u queue full, failed to send item\n", xTaskGetTickCount());
            pthread_mutex_unlock(&xQueue->lock);
            return pdFAIL;
        }

        if (!waitLogged) {
            printf("[FreeRTOS-SIM] tick=%u queue full, producer waits up to %u ticks\n",
                   xTaskGetTickCount(),
                   xTicksToWait);
            waitLogged = true;
        }

        if (pthread_cond_timedwait(&xQueue->notFull, &xQueue->lock, &timeoutTs) == ETIMEDOUT) {
            printf("[FreeRTOS-SIM] tick=%u queue full, send timeout\n", xTaskGetTickCount());
            pthread_mutex_unlock(&xQueue->lock);
            return pdFAIL;
        }
    }

    if (g_stopRequested) {
        pthread_mutex_unlock(&xQueue->lock);
        return pdFAIL;
    }

    memcpy(xQueue->buffer + ((size_t)xQueue->tail * (size_t)xQueue->itemSize),
           pvItemToQueue,
           (size_t)xQueue->itemSize);
    xQueue->tail = (xQueue->tail + 1U) % xQueue->length;
    ++xQueue->size;

    (void)pthread_cond_signal(&xQueue->notEmpty);
    pthread_mutex_unlock(&xQueue->lock);
    return pdPASS;
}

BaseType_t xQueueReceive(QueueHandle_t xQueue, void *pvBuffer, TickType_t xTicksToWait)
{
    struct timespec timeoutTs;
    bool hasTimeout = false;

    if ((xQueue == NULL) || (pvBuffer == NULL)) {
        return pdFAIL;
    }

    pthread_mutex_lock(&xQueue->lock);

    if (xTicksToWait > 0U) {
        pthread_mutex_lock(&g_schedulerLock);
        make_timeout_at(&timeoutTs, xTicksToWait);
        pthread_mutex_unlock(&g_schedulerLock);
        hasTimeout = true;
    }

    while ((xQueue->size == 0U) && !g_stopRequested) {
        if (!hasTimeout) {
            pthread_mutex_unlock(&xQueue->lock);
            return pdFAIL;
        }

        if (pthread_cond_timedwait(&xQueue->notEmpty, &xQueue->lock, &timeoutTs) == ETIMEDOUT) {
            pthread_mutex_unlock(&xQueue->lock);
            return pdFAIL;
        }
    }

    if (g_stopRequested) {
        pthread_mutex_unlock(&xQueue->lock);
        return pdFAIL;
    }

    memcpy(pvBuffer,
           xQueue->buffer + ((size_t)xQueue->head * (size_t)xQueue->itemSize),
           (size_t)xQueue->itemSize);
    xQueue->head = (xQueue->head + 1U) % xQueue->length;
    --xQueue->size;

    (void)pthread_cond_signal(&xQueue->notFull);
    pthread_mutex_unlock(&xQueue->lock);
    return pdPASS;
}

UBaseType_t uxQueueMessagesWaiting(QueueHandle_t xQueue)
{
    UBaseType_t size = 0U;

    if (xQueue == NULL) {
        return 0U;
    }

    pthread_mutex_lock(&xQueue->lock);
    size = xQueue->size;
    pthread_mutex_unlock(&xQueue->lock);

    return size;
}

TimerHandle_t xTimerCreate(const char *pcTimerName,
                           TickType_t xTimerPeriodInTicks,
                           BaseType_t xAutoReload,
                           void *pvTimerID,
                           TimerCallbackFunction_t pxCallbackFunction)
{
    pthread_mutex_lock(&g_schedulerLock);
    for (UBaseType_t i = 0U; i < FREERTOS_SIM_MAX_TIMERS; ++i) {
        if (!g_timers[i].used) {
            if ((pcTimerName == NULL) || (xTimerPeriodInTicks == 0U) || (pxCallbackFunction == NULL)) {
                pthread_mutex_unlock(&g_schedulerLock);
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
            pthread_mutex_unlock(&g_schedulerLock);
            return &g_timers[i];
        }
    }

    pthread_mutex_unlock(&g_schedulerLock);

    return NULL;
}

BaseType_t xTimerStart(TimerHandle_t xTimer, TickType_t xTicksToWait)
{
    (void)xTicksToWait;

    if (xTimer == NULL) {
        return pdFAIL;
    }

    pthread_mutex_lock(&g_schedulerLock);
    xTimer->nextExpiry = g_tick + xTimer->period;
    xTimer->active = true;
    pthread_mutex_unlock(&g_schedulerLock);
    return pdPASS;
}

void vTaskDelay(TickType_t xTicksToDelay)
{
    TickType_t wakeAt = 0U;

    if (g_currentTask == NULL) {
        return;
    }

    pthread_mutex_lock(&g_schedulerLock);
    wakeAt = g_tick + xTicksToDelay;
    g_currentTask->nextRunTick = wakeAt;
    while (!g_stopRequested && (g_tick < wakeAt)) {
        (void)pthread_cond_wait(&g_tickCond, &g_schedulerLock);
    }
    pthread_mutex_unlock(&g_schedulerLock);
}

void vTaskStartScheduler(void)
{
    pthread_mutex_lock(&g_schedulerLock);
    g_stopRequested = false;
    g_schedulerStarted = true;
    for (UBaseType_t i = 0U; i < g_taskCount; ++i) {
        if (g_tasks[i].used && !g_tasks[i].threadStarted) {
            if (pthread_create(&g_tasks[i].thread, NULL, task_thread_main, &g_tasks[i]) == 0) {
                g_tasks[i].threadStarted = true;
            }
        }
    }
    (void)pthread_cond_broadcast(&g_tickCond);
    pthread_mutex_unlock(&g_schedulerLock);

    puts("[FreeRTOS-SIM] Scheduler started");
}

TickType_t xTaskGetTickCount(void)
{
    TickType_t current = 0U;

    pthread_mutex_lock(&g_schedulerLock);
    current = g_tick;
    pthread_mutex_unlock(&g_schedulerLock);

    return current;
}

static void process_timers(void)
{
    TickType_t curTick = 0U;

    for (UBaseType_t i = 0U; i < FREERTOS_SIM_MAX_TIMERS; ++i) {
        TimerCallbackFunction_t callback = NULL;
        void *timerId = NULL;
        const char *timerName = NULL;

        pthread_mutex_lock(&g_schedulerLock);
        curTick = g_tick;
        if (g_timers[i].used && g_timers[i].active && (curTick >= g_timers[i].nextExpiry)) {
            callback = g_timers[i].callback;
            timerId = g_timers[i].id;
            timerName = g_timers[i].name;

            if (g_timers[i].autoReload == pdTRUE) {
                g_timers[i].nextExpiry += g_timers[i].period;
            } else {
                g_timers[i].active = false;
            }
        }
        pthread_mutex_unlock(&g_schedulerLock);

        if (callback != NULL) {
            printf("[FreeRTOS-SIM] tick=%u run timer=%s\n", curTick, timerName);
            callback(timerId);
        }
    }
}

void vFreeRtosSimRunTicks(TickType_t totalTicks, TickType_t tickPeriodMs)
{
    pthread_mutex_lock(&g_schedulerLock);
    if (!g_schedulerStarted) {
        pthread_mutex_unlock(&g_schedulerLock);
        puts("[FreeRTOS-SIM] Scheduler not started");
        return;
    }
    g_tickPeriodMs = (tickPeriodMs == 0U) ? 1U : tickPeriodMs;
    pthread_mutex_unlock(&g_schedulerLock);

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

        for (uint64_t e = 0U; e < expirations; ++e) {
            pthread_mutex_lock(&g_schedulerLock);
            ++g_tick;
            (void)pthread_cond_broadcast(&g_tickCond);
            pthread_mutex_unlock(&g_schedulerLock);
            process_timers();
        }
    }

    (void)close(tfd);
}

void vFreeRtosSimDestroyAll(void)
{
    pthread_mutex_lock(&g_schedulerLock);
    g_stopRequested = true;
    g_schedulerStarted = false;
    (void)pthread_cond_broadcast(&g_tickCond);
    pthread_mutex_unlock(&g_schedulerLock);

    for (UBaseType_t i = 0U; i < FREERTOS_SIM_MAX_QUEUES; ++i) {
        if (g_queues[i] != NULL) {
            pthread_mutex_lock(&g_queues[i]->lock);
            (void)pthread_cond_broadcast(&g_queues[i]->notEmpty);
            (void)pthread_cond_broadcast(&g_queues[i]->notFull);
            pthread_mutex_unlock(&g_queues[i]->lock);
        }
    }

    for (UBaseType_t i = 0U; i < g_taskCount; ++i) {
        if (g_tasks[i].threadStarted) {
            (void)pthread_join(g_tasks[i].thread, NULL);
            g_tasks[i].threadStarted = false;
        }
        g_tasks[i].used = false;
        g_tasks[i].nextRunTick = 0U;
    }

    for (UBaseType_t i = 0U; i < FREERTOS_SIM_MAX_TIMERS; ++i) {
        g_timers[i].used = false;
        g_timers[i].active = false;
    }

    for (UBaseType_t i = 0U; i < FREERTOS_SIM_MAX_QUEUES; ++i) {
        if (g_queues[i] != NULL) {
            (void)pthread_cond_destroy(&g_queues[i]->notFull);
            (void)pthread_cond_destroy(&g_queues[i]->notEmpty);
            (void)pthread_mutex_destroy(&g_queues[i]->lock);
            free(g_queues[i]->buffer);
            free(g_queues[i]);
            g_queues[i] = NULL;
        }
    }

    g_taskCount = 0U;
    g_tick = 0U;
    g_tickPeriodMs = 1U;
    g_stopRequested = false;
}

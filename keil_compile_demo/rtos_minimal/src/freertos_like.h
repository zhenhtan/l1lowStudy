#ifndef FREERTOS_LIKE_H
#define FREERTOS_LIKE_H

#include <stddef.h>
#include <stdint.h>

typedef int BaseType_t;
typedef unsigned int UBaseType_t;
typedef unsigned int TickType_t;

typedef void (*TaskFunction_t)(void *);
typedef void (*TimerCallbackFunction_t)(void *);

typedef struct QueueControl *QueueHandle_t;
typedef struct TimerControl *TimerHandle_t;

#define pdPASS 1
#define pdFAIL 0
#define pdTRUE 1
#define pdFALSE 0

#define pdMS_TO_TICKS(ms) ((TickType_t)(ms))

BaseType_t xTaskCreate(TaskFunction_t pxTaskCode,
                       const char *pcName,
                       uint16_t usStackDepth,
                       void *pvParameters,
                       UBaseType_t uxPriority,
                       void *pxCreatedTask);

QueueHandle_t xQueueCreate(UBaseType_t uxQueueLength, UBaseType_t uxItemSize);
BaseType_t xQueueSend(QueueHandle_t xQueue, const void *pvItemToQueue, TickType_t xTicksToWait);
BaseType_t xQueueReceive(QueueHandle_t xQueue, void *pvBuffer, TickType_t xTicksToWait);
UBaseType_t uxQueueMessagesWaiting(QueueHandle_t xQueue);

TimerHandle_t xTimerCreate(const char *pcTimerName,
                           TickType_t xTimerPeriodInTicks,
                           BaseType_t xAutoReload,
                           void *pvTimerID,
                           TimerCallbackFunction_t pxCallbackFunction);
BaseType_t xTimerStart(TimerHandle_t xTimer, TickType_t xTicksToWait);

void vTaskDelay(TickType_t xTicksToDelay);
void vTaskStartScheduler(void);
TickType_t xTaskGetTickCount(void);

void vFreeRtosSimRunTicks(TickType_t totalTicks, TickType_t tickPeriodMs);
void vFreeRtosSimDestroyAll(void);

#endif

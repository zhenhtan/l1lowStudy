#include "app_tasks.h"

#include <stdio.h>

#include "bsp_led.h"
#include "freertos_like.h"

static QueueHandle_t g_queue = NULL;
static int g_next_value = 100;

BaseType_t app_tasks_init(void)
{
    g_queue = xQueueCreate(2U, sizeof(int));
    if (g_queue == NULL) {
        return pdFAIL;
    }

    g_next_value = 100;
    return pdPASS;
}

void vProducerTask(void *pvParameters)
{
    (void)pvParameters;
    const BaseType_t ok = xQueueSend(g_queue, &g_next_value, pdMS_TO_TICKS(4U));

    if (ok == pdPASS) {
        printf("[PRODUCER] push=%d, qsize=%u\n", g_next_value, uxQueueMessagesWaiting(g_queue));
        ++g_next_value;
    } else {
        puts("[PRODUCER] queue full");
    }

    vTaskDelay(1U);
}

void vConsumerTask(void *pvParameters)
{
    (void)pvParameters;
    int value = 0;
    const BaseType_t ok = xQueueReceive(g_queue, &value, pdMS_TO_TICKS(2U));

    if (ok == pdPASS) {
        printf("[CONSUMER] pop=%d, qsize=%u\n", value, uxQueueMessagesWaiting(g_queue));
        bsp_led_set((value % 2) != 0);
    } else {
        puts("[CONSUMER] queue empty");
    }

    vTaskDelay(4U);
}

void vHeartbeatTimerCallback(void *pvTimerId)
{
    (void)pvTimerId;
    printf("[HEARTBEAT] tick=%u, led=%d\n", xTaskGetTickCount(), bsp_led_get() ? 1 : 0);
}

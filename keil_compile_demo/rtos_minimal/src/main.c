#include <stdio.h>

#include "app_tasks.h"
#include "bsp_led.h"
#include "freertos_like.h"

int main(void)
{
    TimerHandle_t heartbeatTimer = NULL;

    puts("[APP] FreeRTOS API-style demo");
    bsp_led_init();
    if (app_tasks_init() != pdPASS) {
        puts("[APP] init queue failed");
        return 1;
    }

    if (xTaskCreate(vProducerTask, "producer", 256U, NULL, 2U, NULL) != pdPASS) {
        puts("[APP] add producer failed");
        return 1;
    }

    if (xTaskCreate(vConsumerTask, "consumer", 256U, NULL, 1U, NULL) != pdPASS) {
        puts("[APP] add consumer failed");
        return 1;
    }

    heartbeatTimer = xTimerCreate("heartbeat", 5U, pdTRUE, NULL, vHeartbeatTimerCallback);
    if (heartbeatTimer == NULL) {
        puts("[APP] create heartbeat timer failed");
        return 1;
    }

    if (xTimerStart(heartbeatTimer, 0U) != pdPASS) {
        puts("[APP] start heartbeat timer failed");
        return 1;
    }

    vTaskStartScheduler();
    vFreeRtosSimRunTicks(20U, 100U);
    vFreeRtosSimDestroyAll();

    puts("[APP] FreeRTOS API-style demo finished");
    return 0;
}

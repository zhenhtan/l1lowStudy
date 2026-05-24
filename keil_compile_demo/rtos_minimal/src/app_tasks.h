#ifndef APP_TASKS_H
#define APP_TASKS_H

#include "freertos_like.h"

BaseType_t app_tasks_init(void);
void vProducerTask(void *pvParameters);
void vConsumerTask(void *pvParameters);
void vHeartbeatTimerCallback(void *pvTimerId);

#endif

# Minimal FreeRTOS API-style Demo

This is a FreeRTOS naming-style simulation (not a full kernel).

Implemented API style:
- `xTaskCreate`, `vTaskDelay`, `vTaskStartScheduler`
- `xQueueCreate`, `xQueueSend`, `xQueueReceive`
- `xTimerCreate`, `xTimerStart`

## L1LOW concept mapping
- L1LOW `Queue` -> `xQueueCreate` + `xQueueSend/xQueueReceive`
- L1LOW `TimerManager/BaseTimeout` -> `xTimerCreate` + callback
- L1LOW periodic event handling -> task loop + `vTaskDelay`

## Files
- `freertos_like.c`: minimal API-compatible simulation layer
- `app_tasks.c`: producer/consumer tasks and timer callback
- `main.c`: FreeRTOS-style object creation and scheduler run loop

## Build and run
```bash
cd leaningProgram/keil_compile_demo/rtos_minimal
make clean && make run
```

## Expected behavior
- producer task pushes numbers every 2 ticks
- consumer task pops every 3 ticks and updates LED state
- heartbeat timer callback prints status every 5 ticks

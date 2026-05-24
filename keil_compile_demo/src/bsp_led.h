#ifndef BSP_LED_H
#define BSP_LED_H

#include <stdbool.h>

void bsp_led_init(void);
void bsp_led_set(bool on);
bool bsp_led_get(void);

#endif

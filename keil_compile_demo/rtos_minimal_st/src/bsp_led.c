#include "bsp_led.h"

#include <stdio.h>

static bool g_led_on = false;

void bsp_led_init(void)
{
    g_led_on = false;
    puts("[BSP] LED initialized -> OFF");
}

void bsp_led_set(bool on)
{
    g_led_on = on;
    printf("[BSP] LED -> %s\n", g_led_on ? "ON" : "OFF");
}

bool bsp_led_get(void)
{
    return g_led_on;
}

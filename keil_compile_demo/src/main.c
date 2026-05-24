#include <stdio.h>

#include "bsp_led.h"
#include "system_init.h"

int main(void)
{
    system_clock_init();
    bsp_led_init();

    for (int i = 0; i < 6; ++i) {
        const int led_on = (i % 2) == 0;
        bsp_led_set(led_on);
        printf("[APP] loop=%d, led=%d\n", i, bsp_led_get());
        systick_delay_ms(300U);
    }

    puts("[APP] Demo finished");
    return 0;
}

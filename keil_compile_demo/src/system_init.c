#include "system_init.h"

#include <stdio.h>
#include <time.h>

void system_clock_init(void)
{
    puts("[SYS] System clock initialized (simulated)");
}

void systick_delay_ms(unsigned int ms)
{
    const clock_t wait_ticks = (clock_t)((double)ms * (double)CLOCKS_PER_SEC / 1000.0);
    const clock_t start = clock();

    while ((clock() - start) < wait_ticks) {
    }
}

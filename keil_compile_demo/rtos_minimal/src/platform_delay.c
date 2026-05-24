#include "platform_delay.h"

#include <time.h>

void platform_delay_ms(unsigned int ms)
{
    const clock_t wait_ticks = (clock_t)((double)ms * (double)CLOCKS_PER_SEC / 1000.0);
    const clock_t start = clock();

    while ((clock() - start) < wait_ticks) {
    }
}

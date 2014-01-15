#include <stdlib.h>
#include <stdint.h>

#include <libopencm3/cm3/systick.h>

#include "defs.h"

#include "led.h"
#include "timer.h"

volatile unsigned timer[NTIMERS];

void sys_tick_handler(void)
{
    unsigned i;
    int blink;

    for (i = 0; i < NTIMERS; i++)
        if (timer[i] > 0)
            timer[i]--;

    if ((blink = led_blink_state()) && timer[TIMER_LED] == 0) {
        led_toggle(led_blink_type());
        timer[TIMER_LED] = (blink & LED_STATE_RAPID) ? 50 : 200;
    }
}

void wait(unsigned msec)
{
    timer[TIMER_DELAY] = msec;
    while(timer[TIMER_DELAY] > 0);
}

void timers_init(void)
{
    systick_set_clocksource(STK_CTRL_CLKSOURCE_AHB);
    /* systick mhz */
    systick_set_reload(180 * 1000);
    systick_interrupt_enable();
    systick_counter_enable();
}

inline void set_timer(bl_timer_t tim, unsigned int msec)
{
    timer[tim] = msec;
}

inline unsigned int check_timer(bl_timer_t tim)
{
    return timer[tim];
}

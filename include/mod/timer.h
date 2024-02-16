#ifndef __BL_TIMER_H
#define __BL_TIMER_H

#define NTIMERS     5

typedef enum {
    TIMER_USB = 0,
    TIMER_IO,
    TIMER_BL,
    TIMER_DELAY,
    TIMER_LED
} bl_timer_t;

void timers_init(void);
void set_timer(bl_timer_t tim, unsigned int msec);
unsigned int check_timer(bl_timer_t tim);
void wait(unsigned msec);

#endif /* __BL_TIMER_H */

#ifndef __BL_TIMER_H
#define __BL_TIMER_H

#define NTIMERS     5

typedef enum {
    TIMER_USB = 0,
    TIMER_IO,
    TIMER_BL,
    TIMER_DELAY,
    TIMER_LED
} timer_t;

void timers_init(void);
inline void set_timer(timer_t tim, unsigned int msec);
inline unsigned int check_timer(timer_t tim);
inline void delay(unsigned msec);

#endif /* __BL_TIMER_H */

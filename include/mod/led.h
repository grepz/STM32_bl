#ifndef __BL_LED_H
#define __BL_LED_H

#define BOARD_LED_ON     gpio_set
#define BOARD_LED_OFF    gpio_clear

#define LED_AVAILABLE 0

#define LED_STATE_NOBLINK 0
#define LED_STATE_BLINK   1 << 0
#define LED_STATE_RAPID   1 << 1

void led_on(unsigned led);
void led_off(unsigned led);
void led_toggle(unsigned led);
void led_gpio_init(void);

void led_blink(unsigned int type, unsigned int state);
unsigned int led_blink_state(void);
unsigned int led_blink_type(void);

#endif /* __BL_LED_H */

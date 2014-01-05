#ifndef __BL_LED_H
#define __BL_LED_H

#define BOARD_PORT_LEDS GPIOD

#define BOARD_PIN_LED1 GPIO12
#define BOARD_PIN_LED2 GPIO13
#define BOARD_PIN_LED3 GPIO14
#define BOARD_PIN_LED4 GPIO15

#define BOARD_CLOCK_LEDS RCC_AHB1ENR_IOPDEN
#define BOARD_LED_ON     gpio_set
#define BOARD_LED_OFF    gpio_clear

#define LED_ACTIVITY 0
#define LED_ERROR    1
#define LED_BL       2
#define LED_USB      3

#define LED_STATE_BLINK 1 << 0
#define LED_STATE_RAPID 1 << 1

void led_on(unsigned led);
void led_off(unsigned led);
void led_toggle(unsigned led);
void led_gpio_init(void);

inline void led_blink(unsigned int type, unsigned int state);
inline unsigned int led_blink_state(void);
inline unsigned int led_blink_type(void);

#endif /* __BL_LED_H */

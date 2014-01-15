#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

#include "board.h"

#include "defs.h"
#include "led.h"

static unsigned int __type;
static unsigned int __state;

void led_on(unsigned led)
{
    switch (led) {
    case 0:
        BOARD_LED_ON(BL_LEDS_PORT, BL_LED1_PIN);
        break;
    case 1:
        BOARD_LED_ON(BL_LEDS_PORT, BL_LED2_PIN);
        break;
    case 2:
        BOARD_LED_ON(BL_LEDS_PORT, BL_LED3_PIN);
        break;
    case 3:
        BOARD_LED_ON(BL_LEDS_PORT, BL_LED4_PIN);
        break;
    }
}

void led_off(unsigned led)
{
    switch (led) {
    case 0:
        BOARD_LED_OFF(BL_LEDS_PORT, BL_LED1_PIN);
        break;
    case 1:
        BOARD_LED_OFF(BL_LEDS_PORT, BL_LED2_PIN);
        break;
    case 2:
        BOARD_LED_OFF(BL_LEDS_PORT, BL_LED3_PIN);
        break;
    case 3:
        BOARD_LED_OFF(BL_LEDS_PORT, BL_LED4_PIN);
        break;
    }
}

void led_toggle(unsigned led)
{
    switch (led) {
    case 0:
        gpio_toggle(BL_LEDS_PORT, BL_LED1_PIN);
        break;
    case 1:
        gpio_toggle(BL_LEDS_PORT, BL_LED2_PIN);
        break;
    case 2:
        gpio_toggle(BL_LEDS_PORT, BL_LED3_PIN);
        break;
    case 3:
        gpio_toggle(BL_LEDS_PORT, BL_LED4_PIN);
        break;
    }
}

void led_gpio_init(void)
{
    gpio_mode_setup(BL_LEDS_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE,
                    BL_LED1_PIN | BL_LED2_PIN |
                    BL_LED3_PIN | BL_LED4_PIN);
}

inline void led_blink(unsigned int type, unsigned int state)
{
    __type  = type;
    __state = state;
}

inline unsigned int led_blink_state(void)
{
    return __state;
}

inline unsigned int led_blink_type(void)
{
    return __type;
}

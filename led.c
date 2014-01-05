#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

#include "defs.h"
#include "led.h"

static unsigned int __type;
static unsigned int __state;

void led_on(unsigned led)
{
    switch (led) {
    case 0:
        BOARD_LED_ON(BOARD_PORT_LEDS, BOARD_PIN_LED1);
        break;
    case 1:
        BOARD_LED_ON(BOARD_PORT_LEDS, BOARD_PIN_LED2);
        break;
    case 2:
        BOARD_LED_ON(BOARD_PORT_LEDS, BOARD_PIN_LED3);
        break;
    case 3:
        BOARD_LED_ON(BOARD_PORT_LEDS, BOARD_PIN_LED4);
        break;
    }
}

void led_off(unsigned led)
{
    switch (led) {
    case 0:
        BOARD_LED_OFF(BOARD_PORT_LEDS, BOARD_PIN_LED1);
        break;
    case 1:
        BOARD_LED_OFF(BOARD_PORT_LEDS, BOARD_PIN_LED2);
        break;
    case 2:
        BOARD_LED_OFF(BOARD_PORT_LEDS, BOARD_PIN_LED3);
        break;
    case 3:
        BOARD_LED_OFF(BOARD_PORT_LEDS, BOARD_PIN_LED4);
        break;
    }
}

void led_toggle(unsigned led)
{
    switch (led) {
    case 0:
        gpio_toggle(BOARD_PORT_LEDS, BOARD_PIN_LED1);
        break;
    case 1:
        gpio_toggle(BOARD_PORT_LEDS, BOARD_PIN_LED2);
        break;
    case 2:
        gpio_toggle(BOARD_PORT_LEDS, BOARD_PIN_LED3);
        break;
    case 3:
        gpio_toggle(BOARD_PORT_LEDS, BOARD_PIN_LED4);
        break;
    }
}

void led_gpio_init(void)
{
    rcc_peripheral_enable_clock(&RCC_AHB1ENR, BOARD_CLOCK_LEDS);
    gpio_mode_setup(BOARD_PORT_LEDS, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLDOWN,
                    BOARD_PIN_LED1 | BOARD_PIN_LED2 |
                    BOARD_PIN_LED3 | BOARD_PIN_LED4);
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

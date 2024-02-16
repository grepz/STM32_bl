#include <stdint.h>

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>

#include <board.h>
#include <defs.h>

#include <mod/led.h>

static unsigned int __type  = LED_AVAILABLE;
static unsigned int __state = LED_STATE_NOBLINK;

void led_on(unsigned led)
{
    unsigned led_port = BL_LED1_PIN;
    BOARD_LED_ON(BL_LEDS_PORT, led_port);
}

void led_off(unsigned led)
{
    unsigned led_port = BL_LED1_PIN;
    BOARD_LED_OFF(BL_LEDS_PORT, led_port);
}

void led_toggle(unsigned led)
{
    unsigned led_port = BL_LED1_PIN;
    gpio_toggle(BL_LEDS_PORT, led_port);
}

void led_gpio_init(void)
{
    gpio_mode_setup(BL_LEDS_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE,
                    BL_LEDS_ALL);
}

void led_blink(unsigned int type, unsigned int state)
{
    __type  = type;
    __state = state;
}

unsigned int led_blink_state(void)
{
    return __state;
}

unsigned int led_blink_type(void)
{
    return __type;
}

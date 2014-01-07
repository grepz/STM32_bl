// C source code header -*- coding: utf-8 -*-
// Created: [13.16:57 Январь 03 2014]
// Modified: [15.51:24 Январь 07 2014]
// Description:
// Author: Stanislav M. Ivankin
// Email: lessgrep@gmail.com
// Tags: C,stm32
// License: GPLv2

#include <stdlib.h>

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

#include <libopencm3/usb/usbd.h>
#include <libopencm3/usb/cdc.h>
#include <libopencm3/cm3/scb.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/cm3/systick.h>

#include "defs.h"

#include "led.h"
#include "usb.h"
#include "usart.h"
#include "bl.h"
#include "timer.h"

#include "pff/pff.h"

static void __init(void);

void jump_to_app(void)
{
    const uint32_t *app_base = (const uint32_t *)APP_LOAD_ADDRESS;

    if (app_base[0] == 0xffffffff) {
        bl_dbg("App base address is empty.");
        return;
    }

    usbd_stop();
    usart_stop();

    /* Disable all interrupts */
    systick_interrupt_disable();
    systick_counter_disable();

    led_off(LED_BL);
    led_off(LED_ACTIVITY);
    led_off(LED_USB);

    SCB_VTOR = APP_LOAD_ADDRESS;
//    SCB_VTOR = APP_LOAD_ADDRESS & 0x1FFFFF; /* Max 2 MByte Flash*/
    asm volatile ("msr msp, %0"::"g"
                  (*(volatile uint32_t*)APP_LOAD_ADDRESS));
    /* Jump to application */
    (*(void(**)())(APP_LOAD_ADDRESS + 4))();
}

int main(void)
{
    __init();

    usart_start();
    usbd_create();

    /* HW initialized */
    led_on(LED_ACTIVITY);
    bl_dbg("Bootloader started.");
    if (usb_connect()) {
        led_on(LED_USB);
        bl_dbg("USB connected.");
        bootloader();
    } else {
        jump_to_app();
        bootloader();
    }

    return 0;
}

static void __init(void)
{
    timers_init();
    led_gpio_init();
    usart_gpio_init();
    usb_gpio_init();
    /* Pwr control clock */
//    rcc_peripheral_enable_clock(&RCC_APB1ENR, RCC_APB1ENR_PWREN);
}

// C source code header -*- coding: utf-8 -*-
// Created: [13.16:57 Январь 03 2014]
// Modified: [02.39:18 Январь 06 2014]
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

#include "defs.h"

#include "led.h"
#include "usb.h"
#include "usart.h"
#include "bl.h"
#include "timer.h"

static void __init(void);

int main(void)
{
    __init();

    usart_start();
    usbd_create();

    /* HW initialized */
    led_on(LED_ACTIVITY);
    bl_dbg("Bootloader started.");

    while (!usb_connect());
    led_on(LED_USB);

    bootloader();

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

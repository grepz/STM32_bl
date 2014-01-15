// C source code header -*- coding: utf-8 -*-
// Created: [13.16:57 Январь 03 2014]
// Modified: [22.52:12 Январь 15 2014]
// Description:
// Author: Stanislav M. Ivankin
// Email: lessgrep@gmail.com
// Tags: C,stm32
// License: GPLv2

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

//#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

//#include <libopencm3/usb/usbd.h>
//#include <libopencm3/usb/cdc.h>
//#include <libopencm3/cm3/scb.h>
//#include <libopencm3/stm32/usart.h>

#include "defs.h"

#include "board.h"

#include "init.h"

#include "led.h"
#include "usb.h"
#include "usart.h"
#include "spi.h"
#include "at45db.h"

#include "bl.h"

/* SPI2 DMA:
 * RCC_DMA
 * Enable dma interrupts:
 *   SPI1 RX on DMA1 Channel 2
 *   nvic_set_priority(NVIC_DMA1_CHANNEL2_IRQ, 0);
 *   nvic_enable_irq(NVIC_DMA1_CHANNEL2_IRQ);
 *   SPI1 TX on DMA1 Channel 3
 *   nvic_set_priority(NVIC_DMA1_CHANNEL3_IRQ, 0);
 *   nvic_enable_irq(NVIC_DMA1_CHANNEL3_IRQ);
 * Setup DMA: -||-
 * 
 */

int main(void)
{
    bl_init();

    usart_start();
    spi_start();
//    if (usbd_start()) {
//        led_blink(LED_ACTIVITY, LED_STATE_RAPID);
//        for (;;);
//    }

    /* HW initialized */
    led_on(LED_ACTIVITY);
    bl_dbg("Bootloader started.");

    at45db_start();
    for (;;);
#if 0
    int i;
    for (;;) {
        for (i = 0; i < 1000000; i++)
            asm("nop");
        if (usb_connect()) {
            bl_dbg("1");
            led_on(LED_USB);
        } else {
            bl_dbg("0");
            led_off(LED_USB);
        }
    }
#endif
    if (usb_connect()) {
        led_on(LED_USB);
        bl_dbg("USB connected.");
        bootloader();
    } else {
        bl_dbg("No USB.");
        jump_to_app(APP_LOAD_ADDRESS);
    }

    return 0;
}

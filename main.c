// C source code header -*- coding: utf-8 -*-
// Created: [13.16:57 Январь 03 2014]
// Modified: [21.00:08 Январь 08 2014]
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

#include "board.h"

#include "led.h"
#include "usb.h"
#include "usart.h"
#include "bl.h"
#include "timer.h"

#include "pff/pff.h"

#define FLASH_ACR_ICE (1 << 9)
#define FLASH_ACR_DCE (1 << 10)
#define FLASH_ACR_LATENCY_3WS 0x03
#define FLASH_ACR_LATENCY_5WS 0x05

#if 0
typedef struct __bl_rcc_periph
{
    uint8_t status;
    uint32_t rcc_enr;
} bl_rcc_periph_t;

bl_rcc_periph_t rcc_periph[] = {
    {0, RCC_AHB1ENR_IOPAEN},
    {0, RCC_AHB1ENR_IOPBEN},
    {0, RCC_AHB1ENR_IOPCEN},
    {0, RCC_AHB1ENR_IOPDEN},
    {0, RCC_AHB1ENR_IOPEEN},
    {0, RCC_AHB1ENR_IOPFEN},
    {0, RCC_AHB1ENR_IOPGEN},
    {0, RCC_AHB1ENR_IOPHEN},
    {0, RCC_AHB1ENR_IOPIEN},
};
#endif

static void __clock_setup_hsi(const clock_scale_t *clock);
static void __init(void);

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
        bl_dbg("No USB.");
        for (;;);
        jump_to_app(APP_LOAD_ADDRESS);
        bootloader();
    }

    return 0;
}

void rcc_enable(void)
{
    /* LEDs */
    rcc_peripheral_enable_clock(&RCC_AHB1ENR, RCC_AHB1ENR_IOPDEN);

    /* USART */
#ifdef STM32F4_DISCOVERY
    rcc_peripheral_enable_clock(&RCC_AHB1ENR, RCC_AHB1ENR_IOPAEN);
    rcc_peripheral_enable_clock(&RCC_APB1ENR, RCC_APB1ENR_USART2EN);
#else
    rcc_peripheral_enable_clock(&RCC_AHB1ENR, RCC_AHB1ENR_IOPCEN);
    rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_USART6EN);
#endif

    /* USB */
    rcc_peripheral_enable_clock(&RCC_AHB1ENR, RCC_AHB1ENR_IOPAEN);
    rcc_peripheral_enable_clock(&RCC_AHB2ENR, RCC_AHB2ENR_OTGFSEN);
    rcc_peripheral_enable_clock(&RCC_AHB3ENR, RCC_AHB3ENR_FSMCEN);
}

void rcc_disable(void)
{
    /* LEDs */
    rcc_peripheral_disable_clock(&RCC_AHB1ENR, RCC_AHB1ENR_IOPDEN);

    /* USART */
#ifdef STM32F4_DISCOVERY
    rcc_peripheral_disable_clock(&RCC_AHB1ENR, RCC_AHB1ENR_IOPAEN);
    rcc_peripheral_disable_clock(&RCC_APB1ENR, RCC_APB1ENR_USART2EN);
#else
    rcc_peripheral_disable_clock(&RCC_AHB1ENR, RCC_AHB1ENR_IOPCEN);
    rcc_peripheral_disable_clock(&RCC_APB2ENR, RCC_APB2ENR_USART6EN);
#endif

    /* USB */
    rcc_peripheral_disable_clock(&RCC_AHB1ENR, RCC_AHB1ENR_IOPAEN);
    rcc_peripheral_disable_clock(&RCC_AHB2ENR, RCC_AHB2ENR_OTGFSEN);
    rcc_peripheral_disable_clock(&RCC_AHB3ENR, RCC_AHB3ENR_FSMCEN);
}

static void __init(void)
{
#ifdef STM32F4_DISCOVERY
    rcc_clock_setup_hse_3v3(&hse_8mhz_3v3[CLOCK_3V3_120MHZ]);
#else
    clock_scale_t scale = {
        .pllm = 4,
        .plln = 84,
        .pllp = 2,
        .pllq = 7,
        .hpre = RCC_CFGR_HPRE_DIV_NONE,
        .ppre1 = RCC_CFGR_PPRE_DIV_4,
        .ppre2 = RCC_CFGR_PPRE_DIV_2,
        .power_save = 1,
        .flash_config = FLASH_ACR_ICE | FLASH_ACR_DCE | FLASH_ACR_LATENCY_5WS,
        .apb1_frequency = 168000000ul/4,
        .apb2_frequency = 168000000ul/2,
    };

    __clock_setup_hsi(&scale);
#endif

    rcc_enable();

    timers_init();
    led_gpio_init();
    usart_gpio_init();
    usb_gpio_init();
    /* Pwr control clock */
//    rcc_peripheral_enable_clock(&RCC_APB1ENR, RCC_APB1ENR_PWREN);
}

static void __clock_setup_hsi(const clock_scale_t *clock)
{
	/* Enable internal high-speed oscillator. */
	rcc_osc_on(HSI);
	rcc_wait_for_osc_ready(HSI);
	/* Select HSI as SYSCLK source. */
	rcc_set_sysclk_source(RCC_CFGR_SW_HSI);
        rcc_wait_for_sysclk_status(HSI);

	/* Enable/disable high performance mode */
	if (!clock->power_save) {
		pwr_set_vos_scale(0);
	} else {
		pwr_set_vos_scale(1);
	}

        rcc_osc_off(PLL);
        while ((RCC_CR & RCC_CR_PLLRDY) != 0);

	rcc_set_main_pll_hsi(clock->pllm, clock->plln,
                             clock->pllp, clock->pllq);

        rcc_osc_on(PLL);
	/* Enable PLL oscillator and wait for it to stabilize. */
	rcc_wait_for_osc_ready(PLL);

        /*
	 * Set prescalers for AHB, ADC, ABP1, ABP2.
	 * Do this before touching the PLL (TODO: why?).
	 */
	rcc_set_hpre(clock->hpre);
	rcc_set_ppre1(clock->ppre1);
	rcc_set_ppre2(clock->ppre2);

	/* Configure flash settings. */
	flash_set_ws(clock->flash_config);
	/* Select PLL as SYSCLK source. */
	rcc_set_sysclk_source(RCC_CFGR_SW_PLL);
	/* Wait for PLL clock to be selected. */
	rcc_wait_for_sysclk_status(PLL);

	/* Set the peripheral clock frequencies used. */
	rcc_ppre1_frequency = clock->apb1_frequency;
	rcc_ppre2_frequency = clock->apb2_frequency;
}

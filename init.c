#include <stdlib.h>
#include <stdint.h>

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/f4/flash.h>
#include <libopencm3/stm32/f4/pwr.h>

#include <init.h>

#include <mod/led.h>
#include <mod/timer.h>
#include <mod/usart.h>
#include <mod/spi.h>
#include <mod/usb.h>

static void __clock_setup_hsi(const struct rcc_clock_scale *clock);
static void __gpio_init(void);

void bl_init(void)
{
    nvic_enable();
    __clock_setup_hsi(&rcc_hsi_configs[RCC_CLOCK_3V3_96MHZ]);
    rcc_enable();
    __gpio_init();
    timers_init();
}

void rcc_enable(void)
{
    rcc_peripheral_enable_clock(&RCC_APB1ENR, RCC_APB1ENR_SPI2EN);
    rcc_peripheral_enable_clock(&RCC_APB2ENR,
                                RCC_APB2ENR_SYSCFGEN | RCC_APB2ENR_USART6EN);
    rcc_peripheral_enable_clock(&RCC_AHB1ENR,
                                RCC_AHB1ENR_IOPAEN |
                                RCC_AHB1ENR_IOPBEN |
                                RCC_AHB1ENR_IOPCEN |
                                RCC_AHB1ENR_IOPDEN);
    rcc_peripheral_enable_clock(&RCC_AHB2ENR, RCC_AHB2ENR_OTGFSEN);
    rcc_peripheral_enable_clock(&RCC_AHB1ENR, RCC_AHB1ENR_DMA1EN);
}

void rcc_disable(void)
{
    rcc_peripheral_disable_clock(&RCC_APB1ENR, RCC_APB1ENR_SPI2EN);
    rcc_peripheral_disable_clock(&RCC_APB2ENR,
                                 RCC_APB2ENR_SYSCFGEN|RCC_APB2ENR_USART6EN);
    rcc_peripheral_disable_clock(&RCC_AHB1ENR,
                                 RCC_AHB1ENR_IOPAEN|RCC_AHB1ENR_IOPBEN|
                                 RCC_AHB1ENR_IOPCEN|RCC_AHB1ENR_IOPDEN);
    rcc_peripheral_disable_clock(&RCC_AHB2ENR, RCC_AHB2ENR_OTGFSEN);
    rcc_peripheral_disable_clock(&RCC_AHB1ENR, RCC_AHB1ENR_DMA1EN);
}

void nvic_enable(void)
{
    /* USB CDC/ACM */
    nvic_enable_irq(NVIC_OTG_FS_IRQ);
    /* SPI RX */
    nvic_enable_irq(NVIC_DMA1_STREAM3_IRQ);
    /* SPI TX */
    nvic_enable_irq(NVIC_DMA1_STREAM4_IRQ);
}

void nvic_disable(void)
{
    /* SPI RX */
    nvic_disable_irq(NVIC_DMA1_STREAM3_IRQ);
    /* SPI TX */
    nvic_disable_irq(NVIC_DMA1_STREAM4_IRQ);
    /* USB CDC/ACM */
    nvic_disable_irq(NVIC_OTG_FS_IRQ);
}

static void __clock_setup_hsi(const struct rcc_clock_scale *clock)
{
	/* Enable internal high-speed oscillator. */
	rcc_osc_on(RCC_HSI);
	rcc_wait_for_osc_ready(RCC_HSI);
	/* Select HSI as SYSCLK source. */
	rcc_set_sysclk_source(RCC_CFGR_SW_HSI);
    rcc_wait_for_sysclk_status(RCC_HSI);

	/* Enable/disable high performance mode */
#if 0
	if (!clock->power_save) {
		pwr_set_vos_scale(0);
	} else {
        pwr_set_vos_scale(1);
    }
#endif

    rcc_osc_off(RCC_PLL);
    while ((RCC_CR & RCC_CR_PLLRDY) != 0) {
    }

	rcc_set_main_pll_hsi(clock->pllm, clock->plln,
                         clock->pllp, clock->pllq, clock->pllr);

    rcc_osc_on(RCC_PLL);
	/* Enable PLL oscillator and wait for it to stabilize. */
	rcc_wait_for_osc_ready(RCC_PLL);

	rcc_set_hpre(clock->hpre);
	rcc_set_ppre1(clock->ppre1);
	rcc_set_ppre2(clock->ppre2);

	/* Configure flash settings. */
	flash_set_ws(clock->flash_config);
	/* Select PLL as SYSCLK source. */
	rcc_set_sysclk_source(RCC_CFGR_SW_PLL);
	/* Wait for PLL clock to be selected. */
	rcc_wait_for_sysclk_status(RCC_PLL);

	/* Set the peripheral clock frequencies used. */
	rcc_apb1_frequency = clock->apb1_frequency;
	rcc_apb2_frequency = clock->apb2_frequency;
}

static void __gpio_init(void)
{
    led_gpio_init();
//    usart_gpio_init();
//    spi_gpio_init();
//    usb_gpio_init();
}

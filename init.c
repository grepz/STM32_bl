#include <stdlib.h>
#include <stdint.h>

#include <libopencm3/cm3/nvic.h>

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/f4/flash.h>
#include <libopencm3/stm32/f4/pwr.h>

#include <init.h>

#include <mod/led.h>
#include <mod/timer.h>
#include <mod/usart.h>
#include <mod/spi.h>
#include <mod/usb.h>

static void __clock_setup_hsi(const clock_scale_t *clock);
static void __gpio_init(void);

void bl_init(void)
{
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

    nvic_enable();
    __clock_setup_hsi(&scale);
    rcc_enable();
    __gpio_init();
    timers_init();
}

void rcc_enable(void)
{
    rcc_peripheral_enable_clock(&RCC_APB1ENR, RCC_APB1ENR_SPI2EN);
    rcc_peripheral_enable_clock(&RCC_APB2ENR,
                                 RCC_APB2ENR_SYSCFGEN|RCC_APB2ENR_USART6EN);
    rcc_peripheral_enable_clock(&RCC_AHB1ENR,
                                 RCC_AHB1ENR_IOPAEN|RCC_AHB1ENR_IOPBEN|
                                 RCC_AHB1ENR_IOPCEN|RCC_AHB1ENR_IOPDEN);
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

static void __gpio_init(void)
{
    led_gpio_init();
    usart_gpio_init();
    spi_gpio_init();
    usb_gpio_init();
}

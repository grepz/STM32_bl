#include <stdlib.h>
#include <string.h>

#include <libopencm3/stm32/f4/rcc.h>
#include <libopencm3/stm32/f4/gpio.h>
#include <libopencm3/stm32/usart.h>

#include <libopencm3/cm3/nvic.h>

#include "defs.h"

#include "usart.h"
#include "led.h"

void usart_gpio_init(void)
{
    rcc_peripheral_enable_clock(&RCC_AHB1ENR, RCC_AHB1ENR_IOPAEN);
    rcc_peripheral_enable_clock(&RCC_APB1ENR, BL_USART_RCC);

    gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO2); /* Tx */
    gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO3); /* Rx */
    gpio_set_output_options(GPIOA, GPIO_OTYPE_OD, GPIO_OSPEED_25MHZ, GPIO3);

    gpio_set_af(GPIOA, GPIO_AF7, GPIO2);
    gpio_set_af(GPIOA, GPIO_AF7, GPIO3);

//    nvic_enable_irq(NVIC_USART2_IRQ);
}

void usart_start(void)
{
    /* USART configuration */
    usart_set_baudrate(BL_USART, 115200);
    usart_set_databits(BL_USART, 8);
    usart_set_stopbits(BL_USART, USART_STOPBITS_1);
    usart_set_mode(BL_USART, USART_MODE_TX_RX);
    usart_set_parity(BL_USART, USART_PARITY_NONE);
    usart_set_flow_control(BL_USART, USART_FLOWCONTROL_NONE);

    /* Enable interrupts */
//    usart_enable_rx_interrupt(USART2);
    usart_enable(BL_USART);
}

void usart_stop(void)
{
//    usart_disable_rx_interrupt(USART2);
    usart_disable(BL_USART);
}

void usart_print(const uint8_t *data, size_t len)
{
    while (len--)
        usart_send_blocking(BL_USART, *data++);
}

void print(const char *msg)
{
    usart_print((const uint8_t *)msg, strlen(msg));
}


#if 0
void usart2_isr(void)
{
    static uint8_t data = 'A';

    /* RXNE. */
    if (((USART_CR1(BL_USART) & USART_CR1_RXNEIE) != 0) &&
        ((USART_SR(BL_USART) & USART_SR_RXNE) != 0)) {
        data = usart_recv(BL_USART);
        usart_enable_tx_interrupt(BL_USART);
    }
    /* TXE. */
    if (((USART_CR1(BL_USART) & USART_CR1_TXEIE) != 0) &&
        ((USART_SR(BL_USART) & USART_SR_TXE) != 0)) {
        usart_send(BL_USART, data);
        usart_disable_tx_interrupt(BL_USART);
    }
}
#endif

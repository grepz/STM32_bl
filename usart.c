#include <stdlib.h>
#include <string.h>

#include <libopencm3/stm32/f4/rcc.h>
#include <libopencm3/stm32/f4/gpio.h>
#include <libopencm3/stm32/usart.h>

#include <libopencm3/cm3/nvic.h>

#include "board.h"

#include "defs.h"

#include "usart.h"
#include "led.h"

void usart_gpio_init(void)
{
    gpio_mode_setup(SERIAL_TX_PORT, GPIO_MODE_AF,
                    GPIO_PUPD_NONE, SERIAL_TX_PIN); /* Tx */
    gpio_mode_setup(SERIAL_RX_PORT, GPIO_MODE_AF,
                    GPIO_PUPD_NONE, SERIAL_RX_PIN); /* Rx */
    gpio_set_output_options(SERIAL_RX_PORT, GPIO_OTYPE_OD,
                            GPIO_OSPEED_25MHZ, SERIAL_RX_PIN);

    gpio_set_af(SERIAL_TX_PORT, USART_AF, SERIAL_TX_PIN);
    gpio_set_af(SERIAL_RX_PORT, USART_AF, SERIAL_RX_PIN);
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

    usart_enable(BL_USART);
}

void usart_stop(void)
{
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

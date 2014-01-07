#ifndef __BL_USART_H
#define __BL_USART_H

#include <stdlib.h>

void usart_gpio_init(void);
void usart_start(void);
void usart_stop(void);
void usart_print(const uint8_t *data, size_t len);
void print(const char *msg);

#endif /* __BL_USART_H */

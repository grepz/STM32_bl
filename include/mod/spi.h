#ifndef __BL_SPI_H
#define __BL_SPI_H

#include <libopencm3/stm32/spi.h>

void spi_gpio_init(void);
void spi_start(void);
void spi_stop(void);
inline void spi_select(uint8_t select);
void spi_exchange_nodma(const uint8_t *txbuf, uint8_t *rxbuf, size_t n);
void spi_exchange_dma(const uint8_t *txbuf, uint8_t *rxbuf, size_t n);
int spi_dma_transceive(uint8_t *txbuf, size_t txlen,
                       uint8_t *rxbuf, size_t rxlen);

#endif /* __BL_SPI_H */

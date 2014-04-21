#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/dma.h>

#include <defs.h>
#include <board.h>

#include <mod/spi.h>
#include <mod/timer.h>

static volatile int rxcomplete = 0;
static volatile int txcomplete = 0;

static void __dma_rx_setup(uint8_t *rxb, uint8_t *_rxb, size_t n);
static void __dma_tx_setup(const uint8_t *txb, const uint8_t *_txb, size_t n);
static void __dma_rx_wait(void);
static void __dma_tx_wait(void);

static void __spi_dma_start(uint32_t stream);
static void __spi_dma_setup(uint32_t stream, uint32_t maddr, size_t n);

void spi_gpio_init(void)
{
    /* SCK, MISO, MOSI */
    gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE,
                    BL_SPI2_SCK | BL_SPI2_MISO | BL_SPI2_MOSI);
    gpio_set_af(GPIOB, GPIO_AF5,
                    BL_SPI2_SCK | BL_SPI2_MISO | BL_SPI2_MOSI);
    /* WriteProtect and Reset */
    gpio_mode_setup(GPIOD, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP,
                    BL_SPI2_RST | BL_SPI2_WP);
    gpio_set_output_options(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,
                            BL_SPI2_RST | BL_SPI2_WP);
    /* SPI select PIN(CS/NSS) */
    gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, BL_SPI2_NSS);
    gpio_set_output_options(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,
                            BL_SPI2_NSS);
}

void spi_start(void)
{
    /* Reset flash chip */
    gpio_clear(GPIOD, BL_SPI2_RST);
    wait(10);
    gpio_set(GPIOD, BL_SPI2_RST);
    wait(10);

    gpio_set(GPIOD, BL_SPI2_WP);
    /* No WriteProtect, Set Chip select to 1(no select) */
    gpio_set(GPIOB, BL_SPI2_NSS);

    /* Reset and disable SPI */
    spi_reset(SPI2);

    /* Disable I2S */
    SPI2_I2SCFGR = 0;

    /* CR1 */
    spi_set_clock_phase_0(SPI2);                /* CPHA = 0    */
    spi_set_clock_polarity_0(SPI2);             /* CPOL = 0    */
    spi_send_msb_first(SPI2);                   /* LSB = 0     */
    spi_set_full_duplex_mode(SPI2);             /* RXONLY = 0  */
    spi_set_unidirectional_mode(SPI2);          /* BIDI = 0    */
    spi_enable_software_slave_management(SPI2); /* SSM = 1     */
    spi_set_nss_high(SPI2);                     /* SSI = 1     */
    spi_set_master_mode(SPI2);                  /* MSTR = 1    */
    spi_set_dff_8bit(SPI2);                     /* DFf = 8 bit */
//    spi_enable_crc(SPI2);
    /* XXX: Too fast? Maybe DIV_4 will be better? */
    spi_set_baudrate_prescaler(SPI2, SPI_CR1_BR_FPCLK_DIV_2);

    /* CR2 */
    spi_enable_ss_output(SPI2); /* SSOE = 1 */
    /* Disable regular interrupt flags */
    spi_disable_tx_buffer_empty_interrupt(SPI2);
    spi_disable_rx_buffer_not_empty_interrupt(SPI2);

    spi_disable_error_interrupt(SPI2);

    /* Enabling RX/TX DMA flags */
    spi_enable_tx_dma(SPI2);
    spi_enable_rx_dma(SPI2);

    d_print("REG: %lu:%lu\r\n", SPI_CR1(SPI2), SPI_CR2(SPI2));

    spi_enable(SPI2);
}

void spi_stop(void)
{
    spi_disable_tx_dma(SPI2);
    spi_disable_rx_dma(SPI2);
    spi_disable(SPI2);
//    spi_clean_disable(SPI2);
}

inline void spi_select(uint8_t select)
{
    /* Selected(1): Low level; De-selected(0): High Level */
    (select == 0) ? gpio_set(GPIOB,BL_SPI2_NSS) : gpio_clear(GPIOB,BL_SPI2_NSS);
}

/* TODO: CHeck if memory is DMA capable */
void spi_exchange_dma(const uint8_t *txb, uint8_t *rxb, size_t n)
{
    static uint8_t _rxb = 0xFF;
    static const uint8_t _txb = 0xFF;

    __dma_rx_setup(rxb, &_rxb, n);
    __dma_tx_setup(txb, &_txb, n);

    __spi_dma_start(DMA_STREAM3); /* RX */
    __spi_dma_start(DMA_STREAM4); /* TX */

    __dma_tx_wait();
    __dma_rx_wait();
}

void spi_exchange_nodma(const uint8_t *txbuf, uint8_t *rxbuf, size_t n)
{
    uint8_t byte;
    const uint8_t *txptr = txbuf;
    uint8_t *rxptr = rxbuf;

    while (n-- > 0) {
        if (txptr)
            byte = *txptr++;
        else
            byte = 0xFF;

        while (!(SPI_SR(SPI2) & SPI_SR_TXE));
        byte = (uint8_t)spi_xfer(SPI2, byte);

        if (rxptr)
            *rxptr++ = byte;
    }
}

static void __dma_rx_setup(uint8_t *rxb, uint8_t *_rxb, size_t n)
{
    uint8_t *bufptr;

    /* Reset stream in case any op is going on */
    dma_stream_reset(DMA1, DMA_STREAM3);

    dma_set_transfer_mode(DMA1,   DMA_STREAM3, DMA_SxCR_DIR_PERIPHERAL_TO_MEM);
    dma_set_priority(DMA1,        DMA_STREAM3, DMA_SxCR_PL_VERY_HIGH);
    dma_set_memory_size(DMA1,     DMA_STREAM3, DMA_SxCR_MSIZE_8BIT);
    dma_set_peripheral_size(DMA1, DMA_STREAM3, DMA_SxCR_PSIZE_8BIT);

    if (rxb) {
        dma_enable_memory_increment_mode(DMA1, DMA_STREAM3);
        bufptr = rxb;
    } else {
        bufptr = _rxb;
    }

    __spi_dma_setup(DMA_STREAM3, (uint32_t)bufptr, n);
}

static void __dma_tx_setup(const uint8_t *txb, const uint8_t *_txb, size_t n)
{
    const uint8_t *bufptr;

    /* Reset stream in case any op is going on */
    dma_stream_reset(DMA1, DMA_STREAM4);

    dma_set_transfer_mode(DMA1,   DMA_STREAM4, DMA_SxCR_DIR_MEM_TO_PERIPHERAL);
    dma_set_priority(DMA1,        DMA_STREAM4, DMA_SxCR_PL_HIGH);
    dma_set_memory_size(DMA1,     DMA_STREAM4, DMA_SxCR_MSIZE_8BIT);
    dma_set_peripheral_size(DMA1, DMA_STREAM4, DMA_SxCR_PSIZE_8BIT);
    if (txb) {
        dma_enable_memory_increment_mode(DMA1, DMA_STREAM4);
        bufptr = txb;
    } else {
        bufptr = _txb;
    }

    __spi_dma_setup(DMA_STREAM4, (uint32_t)bufptr, n);
}

static void __dma_rx_wait(void)
{
    while (rxcomplete != 1);
    rxcomplete = 0;
}

static void __dma_tx_wait(void)
{
    while (txcomplete != 0)
    txcomplete = 0;
}

static void __spi_dma_setup(uint32_t stream, uint32_t maddr, size_t n)
{
    while ((DMA1_SCR(stream) & DMA_SxCR_EN) != 0)
        bl_dbg("Stream enabled");
    /* Set peripheral address */
    dma_set_peripheral_address(DMA1, stream, (uint32_t)&SPI2_DR);
    /* Set memory address */
    dma_set_memory_address(DMA1, stream, maddr);
    /* Set total number of bytes to transfer */
    dma_set_number_of_data(DMA1, stream, n);
    /* Select channel, it is 0 for both RX and TX */
    dma_channel_select(DMA1, stream, DMA_SxCR_CHSEL_0);
}

static void __spi_dma_start(uint32_t stream)
{
    /* XXX: Possible spurious interrupts on good transfers on FEIF? */
    dma_disable_fifo_error_interrupt(DMA1, stream);
    dma_disable_half_transfer_interrupt(DMA1, stream);

    dma_enable_transfer_complete_interrupt(DMA1, stream);
    dma_enable_transfer_error_interrupt(DMA1,    stream);
    dma_enable_transfer_complete_interrupt(DMA1, stream);
    dma_enable_stream(DMA1, stream);
}

/* RX IRQ */
void dma1_stream3_isr(void)
{
//    d_print("RX ISR: %lu:%lu\r\n", DMA1_LISR, DMA1_HISR);

    dma_disable_transfer_complete_interrupt(DMA1, DMA_STREAM3);
    dma_disable_stream(DMA1, DMA_STREAM3);

    rxcomplete = 1;
}

/* TX IRQ */
void dma1_stream4_isr(void)
{
//    d_print("TX ISR: %lu:%lu\r\n", DMA1_LISR, DMA1_HISR);

    dma_stream_reset(DMA1, DMA_STREAM4);
    dma_disable_stream(DMA1, DMA_STREAM4);

    txcomplete = 1;
}

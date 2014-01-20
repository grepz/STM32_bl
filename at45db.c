#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include <libopencm3/stm32/gpio.h>

#include "defs.h"
#include "board.h"

#include "mod/timer.h"
#include "mod/at45db.h"
#include "mod/spi.h"

#define PG_PER_BLOCK        (8)
#define PG_PER_SECTOR       (256)

#define AT45DB_RDDEVID 0x9f /* Product ID, Manufacturer ID and density read */
#define AT45DB_RDSR    0xd7 /* SR */
#define AT45DB_RESUME  0xab /* Resume flash */

#define AT45DB_MANUFACT_ATMEL 0x1F /* Atmel manufacturer */
#define AT45DB_DENSITY_MSK    0x1f /* Density Mask */
#define AT45DB_FAM_MSK        0xe0 /* Family Mask */
#define AT45DB_FAM_DATAFLASH  0x20 /* DataFlash device */
#define AT45DB_DENSITY_64MBIT 0x08 /* AT45DB641 */
/* Read */
#define AT45DB_RDARRAYHF 0x0b /* Continuous array read (high frequency) */
/* Erase */
#define AT45DB_PGERASE   0x81 /* Page Erase */
#define AT45DB_MNTHRUBF1 0x82 /* Main memory page program through buffer 1 */

/* PageSize reporgramming sequence */
#define AT45DB_PGSIZE_SIZE 4
static const uint8_t at45db_pgsize_cmd[] = {
    0x3D, 0x2A, 0x80, 0xA6
};

#define AT45DB_STATUS_PGSIZE  (1 << 0) /* PAGE SIZE */
#define AT45DB_STATUS_PROTECT (1 << 1) /* PROTECT */
#define AT45DB_STATUS_COMP    (1 << 6) /* COMP */
#define AT45DB_STATUS_READY   (1 << 7) /* RDY/BUSY */

static flash_storage_t __flash;

static inline uint8_t __at45db_rdsr(void);
static inline uint8_t __at45db_bsy(void);
static inline void __at45db_page_write(const uint8_t *buf, size_t pg_num);
static inline void __at45db_page_erase(uint32_t sector);

int at45db_start(void)
{
    uint8_t devid[] = {0x0, 0x0, 0x0};

    bl_dbg("Starting AT45DB.");

    spi_select(0);

    spi_select(1);
    /* In case of deep power down */
    spi_xfer(SPI2, AT45DB_RESUME);
    spi_select(0);

    wait(50); /* Actually should be >35 ns */

    /* See `Manufacturer and Device ID Read' topic in manual(pg. 25) */
    spi_select(1);
    spi_xfer(SPI2, AT45DB_RDDEVID);
    spi_exchange_dma(NULL, devid, 3);
    spi_select(0);

    if (devid[0]                        != AT45DB_MANUFACT_ATMEL ||
        (devid[1] & AT45DB_FAM_MSK)     != AT45DB_FAM_DATAFLASH  ||
        (devid[1] & AT45DB_DENSITY_MSK) != AT45DB_DENSITY_64MBIT)
        return -1;

    /* Reprogram flash page size to power of 2.
     * See `"Power of 2" Binary Page Size Option' topic in manual(pg. 25)
     */
    if (!(__at45db_bsy() & AT45DB_STATUS_PGSIZE)) {
        bl_dbg("Repogramming flash page size to be power of 2");
        spi_select(1);
        spi_exchange_dma(at45db_pgsize_cmd, NULL, AT45DB_PGSIZE_SIZE);
        spi_select(0);
    }

    /* TODO: Add other densities, atm only AT45DB64 */
    __flash.pg_shifts     = 10;
    __flash.pg_num        = 8192;
    __flash.block_sz      = 1 << __flash.pg_shifts;
    __flash.erase_sz      = __flash.block_sz;
    __flash.n_eraseblocks = __flash.pg_num;

    d_print("Number of pages: %d; Page shifts: %d\r\n",
            __flash.pg_num, __flash.pg_shifts);

    return 0;
}

inline void at45db_chip_erase(void)
{
    uint8_t cmd[4];

    cmd[0] = 0xc7;
    cmd[1] = 0x94;
    cmd[2] = 0x80;
    cmd[3] = 0x9a;

    spi_select(1);
    spi_exchange_dma(cmd, NULL, 4);
    spi_select(0);

    __at45db_bsy();
}

inline ssize_t at45db_read(uint32_t off, size_t nbytes, uint8_t *buf)
{
  uint8_t cmd[5];

  cmd[0] = AT45DB_RDARRAYHF;
  cmd[1] = (off >> 16) & 0xff;
  cmd[2] = (off >> 8) & 0xff;
  cmd[3] = off & 0xff;
  cmd[4] = 0;

  spi_select(1);
  spi_exchange_dma(cmd, NULL, 5);
  spi_exchange_dma(NULL, buf, nbytes);
  spi_select(0);

  return nbytes;
}

int at45db_erase(uint32_t sblock, size_t n)
{
    size_t pgs;

    pgs = n;

    while (pgs-- > 0) {
        __at45db_page_erase(sblock);
        sblock++;
    }

    return n;
}

inline ssize_t at45db_bread(uint32_t sblock, size_t nblocks, uint8_t *buf)
{
    ssize_t nb;

    d_print("Read at offset=%lu bytes=%lu\r\n",
            sblock << __flash.pg_shifts, nblocks << __flash.pg_shifts);
    nb = at45db_read(sblock << __flash.pg_shifts,
                     nblocks << __flash.pg_shifts, buf);
    if (nb > 0)
        return nb >> __flash.pg_shifts;

    return nb;
}

inline ssize_t at45db_bwrite(uint32_t sblock, size_t nblocks,const uint8_t *buf)
{
    size_t pgs = nblocks;

    while (pgs-- > 0) {
        __at45db_page_write(buf, sblock);
        sblock++;
    }

    return nblocks;
}

static inline uint8_t __at45db_bsy(void)
{
    uint8_t sr;

    while (!((sr = __at45db_rdsr()) & AT45DB_STATUS_READY))
        wait(10);

    return sr;
}

static inline uint8_t __at45db_rdsr(void)
{
    uint8_t status;

    spi_select(1);
    spi_xfer(SPI2, AT45DB_RDSR);
    status = spi_xfer(SPI2, 0xFF);
    spi_select(0);

    return status;
}

static inline void __at45db_page_write(const uint8_t *buf, size_t pg_num)
{
    uint8_t cmd[4];
    uint32_t off;

    off = pg_num << __flash.pg_shifts;

    cmd[0] = AT45DB_MNTHRUBF1;
    cmd[1] = (off >> 16) & 0xff;
    cmd[2] = (off >> 8)  & 0xff;
    cmd[3] = off & 0xff;

    spi_select(1);
    spi_exchange_dma(cmd, NULL, 4);
    spi_exchange_dma(buf, NULL, 1 << __flash.pg_shifts);
    spi_select(0);

    __at45db_bsy();
}

static inline void __at45db_page_erase(uint32_t sector)
{
    uint8_t cmd[4];
    uint32_t off;

    off = sector << __flash.pg_shifts;

    cmd[0] = AT45DB_PGERASE;
    cmd[1] = (off >> 16) & 0xff;
    cmd[2] = (off >> 8) & 0xff;
    cmd[3] = off & 0xff;

    spi_select(1);
    spi_exchange_dma(cmd, NULL, 4);
    spi_select(0);

    __at45db_bsy();
}

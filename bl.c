#include <stdlib.h>
#include <string.h>

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/scb.h>

#include "defs.h"

#include "bl.h"
#include "usart.h"
#include "led.h"
#include "usb.h"
#include "protocol.h"
#include "timer.h"
#include "flash.h"
#include "utils.h"

typedef enum {
    BL_STATE_NONE,
    BL_STATE_SESSION,
    BL_STATE_ERASE,
    BL_STATE_FLASH,
    BL_STATE_FLASH_DATA,
    BL_STATE_DATA_CRC,
    BL_STATE_EOS
} bl_state_t;

static uint8_t board_id[] = {0xFF, 0x11, 0xFF, 0x11};

static inline void __send_handshake(void);
static inline void __send_status(int status);
static inline void __send_data_crc32(uint32_t crc);

static inline int __read_data(uint8_t *data, size_t sz);

//static void do_jump(uint32_t stacktop, uint32_t entrypoint);

void bootloader(void)
{
    unsigned int i;
    unsigned int ss, es;
    int state = BL_STATE_NONE, status;
    uint8_t buf[128];
    uint32_t daddr = 0, dsz = 0, caddr, csz, w;
    data_buf_t dbuf;
    uint32_t data_crc;

    for (;;) {
        if (state == BL_STATE_NONE) {
            daddr = dsz = caddr = csz = w = 0;
            led_off(LED_BL);
        } else
            led_on(LED_BL);

        if (get_byte(buf, 1000) == -1) {
            /* Check inactivity timer if no data present,
             * kill session if timeout
             */
            if (check_timer(TIMER_BL) == 0)
                state = BL_STATE_NONE;
            continue;
        } else {
            /* Reset inactivity timer */
            set_timer(TIMER_BL, BL_TIMEOUT);
        }

        if (state == BL_STATE_NONE) {
            led_off(LED_BL);
            if (*buf != BL_PROTO_CMD_HANDSHAKE || /* Waiting for handshake */
                __read_data(buf + 1, 2) == -1  || /* 3 bytes */
                crc8(buf, 2) != buf[2]) {         /* Checking CRC */
                continue;
            }

            __send_handshake();
            bl_dbg("Handshaking.");
            state = BL_STATE_SESSION;
            continue;
        }

        switch (*buf) {
        case BL_PROTO_CMD_ERASE:
            bl_dbg("Erase command.");
            if (__read_data(buf + 1, 3) == -1)
                continue;

            if (crc8(buf, 3) != buf[3]) {
                status = BL_PROTO_STATUS_CRCERR;
            } else {
                flash_unlock();
                bl_flash_erase_sector(buf[1]);
                flash_lock();
                status = BL_PROTO_STATUS_OK;
            }

            __send_status(status);
            break;
        case BL_PROTO_CMD_FLASH:
            bl_dbg("Flash command.");
            if (__read_data(buf + 1, 10) == -1) {
                status = BL_PROTO_STATUS_READERR;
            } else if (crc8(buf, 10) != buf[10]) {
                status = BL_PROTO_STATUS_CRCERR;
            } else {
                /* Start address */
                daddr |= buf[1];       daddr |= buf[2] << 8;
                daddr |= buf[3] << 16; daddr |= buf[4] << 24;
                daddr -= STM32_BASE_ADDR;
                /* Data size */
                dsz |= buf[5];       dsz |= buf[6] << 8;
                dsz |= buf[7] << 16; dsz |= buf[8] << 24;
                /* Checking if we are in address boundaries */
                if ((daddr + dsz) > APP_SIZE_MAX)
                    status = BL_PROTO_STATUS_ARGERR;
                else
                    status = BL_PROTO_STATUS_OK;
            }

            if (status == BL_PROTO_STATUS_OK &&
                bl_flash_get_sector_num(daddr, dsz, &ss, &es) != -1) {
                i = ss;
                flash_unlock();
                do  {
                    bl_dbg("Erasing sector...");
                    bl_flash_erase_sector(i);
                } while (++i < (es - ss));
                flash_lock();
            }

            __send_status(status);
            break;
        case BL_PROTO_CMD_FLASH_DATA:
            bl_dbg("Flash data command.");
            if (__read_data(buf + 1, 1) == -1) {
                /* Failed reading data size argument */
                status = BL_PROTO_STATUS_READERR;
            } else if (!buf[1] || (buf[1] + 1) % 4) {
                /* Zero size or not word aligned */
                status = BL_PROTO_STATUS_ARGERR;
            } else {
                /* Reset data buffer and read incoming data */
                memset(&dbuf, 0, sizeof(dbuf));
                __read_data(dbuf.b, (unsigned int)buf[1] + 1);

                if (__read_data(buf + 2, 2) == -1) {
                    /* Failed reading command EOM and CRC8 bytes */
                    status = BL_PROTO_STATUS_CRCERR;
                } else {
                    if (crc8(buf, 3) != buf[3]) {
                        /* Command crc check failed */
                        status = BL_PROTO_STATUS_CRCERR;
                    } else {
                        flash_unlock();
                        /* Starting to flash data */
                        for (i = 0; i < ((unsigned int)buf[1]+1)/4; i++) {
                            bl_flash_write_word(daddr, dbuf.w[i]);
                            if (bl_flash_read_word(daddr) != dbuf.w[i]) {
                                /* I/O error */
                                status = BL_PROTO_STATUS_IOERR;
                                break;
                            }

                            daddr += 4;
                        }
                        flash_lock();

                        status = BL_PROTO_STATUS_OK;
                    }
                }
            }

            __send_status(status);
            break;
        case BL_PROTO_CMD_DATA_CRC:
            bl_dbg("CRC check command.");
            if (__read_data(buf + 1, 10) == -1) {
                status = BL_PROTO_STATUS_READERR;
            } else if (crc8(buf, 10) != buf[10]) {
                status = BL_PROTO_STATUS_CRCERR;
            } else {
                caddr = csz = 0;
                /* Start address */
                caddr |= buf[1];       caddr |= buf[2] << 8;
                caddr |= buf[3] << 16; caddr |= buf[4] << 24;
                caddr -= STM32_BASE_ADDR;
                /* Data size */
                csz |= buf[5];       csz |= buf[6] << 8;
                csz |= buf[7] << 16; csz |= buf[8] << 24;
                if ((caddr + csz) > APP_SIZE_MAX) {
                    /* TODO: Check address and size here */
                    status = BL_PROTO_STATUS_ARGERR;
                } else {
                    status = BL_PROTO_STATUS_OK;
                }
            }

            if (status == BL_PROTO_STATUS_OK) {
                data_crc = 0;
                for (i = 0; i < csz/4; i++) {
                    w = bl_flash_read_word(caddr);
                    data_crc = crc32((uint8_t *)&w, 4, data_crc);
                    caddr += 4;
                }

                __send_data_crc32(data_crc);
                jump_to_app();
            } else
                __send_data_crc32(0);
            break;
        case BL_PROTO_CMD_EOS:
            bl_dbg("EOS command.");
            if (__read_data(buf + 1, 2) == -1)
                continue;

            if (crc8(buf, 2) != buf[2]) {
                status = BL_PROTO_STATUS_CRCERR;
            } else
                status = BL_PROTO_STATUS_OK;

            __send_status(status);
            if (status == BL_PROTO_STATUS_OK)
                state = BL_STATE_NONE;
            break;
        }
    }
}


static inline void __send_handshake(void)
{
    uint8_t size[4];

    size[0] = APP_SIZE_MAX & 0xFF;
    size[1] = (APP_SIZE_MAX >> 8)  & 0xFF;
    size[2] = (APP_SIZE_MAX >> 16) & 0xFF;
    size[3] = (APP_SIZE_MAX >> 24) & 0xFF;

    uint8_t msg[] = {
        BL_PROTO_VER, BL_PROTO_REV,
        board_id[0], board_id[1], board_id[2], board_id[3],
        size[0], size[1], size[2], size[3],
        BL_PROTO_EOM, 0};

    msg[11] = crc8(msg, 11);
    usb_msgsend(msg, 12);
}

static inline void __send_data_crc32(uint32_t crc)
{
    uint8_t msg[6];

    msg[0] = crc & 0xFF;
    msg[1] = (crc >> 8) & 0xFF;
    msg[2] = (crc >> 16) & 0xFF;
    msg[3] = (crc >> 24) & 0xFF;
    msg[4] = BL_PROTO_EOM;
    msg[5] = crc8(msg, 5);

    usb_msgsend(msg, 6);
}

static inline void __send_status(int status)
{
    uint8_t msg[] = {status, BL_PROTO_EOM, 0};
    msg[2] = crc8(msg, 2);

    usb_msgsend(msg, 3);
}

static inline int __read_data(uint8_t *data, size_t sz)
{
    while (sz--) {
        if (get_byte(data, 1000) == -1)
            return -1;
        data++;
    }

    return 0;
}

#if 0
static void do_jump(uint32_t stacktop, uint32_t entrypoint)
{
    asm volatile(
        "msr msp, %0	\n"
        "bx	%1	\n"
        : : "r" (stacktop), "r" (entrypoint) : );
    for (;;);
}
#endif

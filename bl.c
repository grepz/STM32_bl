#include <stdlib.h>
#include <string.h>

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/scb.h>
#include <libopencm3/cm3/systick.h>

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
static inline void __send_data_crc();

static inline int __read_data(uint8_t *data, size_t sz);

//static void do_jump(uint32_t stacktop, uint32_t entrypoint);
#if 0
void jump_to_app(void)
{
    const uint32_t *app_base = (const uint32_t *)APP_LOAD_ADDRESS;

    if (app_base[0] == 0xffffffff) {
        bl_dbg("App base address is empty.");
        return;
    }

    for (;;);

    usbd_stop();
    usart_stop();

    /* Disable all interrupts */
    systick_interrupt_disable();
    systick_counter_disable();

    led_on(LED_BL);
    led_off(LED_ACTIVITY);

    SCB_VTOR = APP_LOAD_ADDRESS;
//    SCB_VTOR = APP_LOAD_ADDRESS & 0x1FFFFF; /* Max 2 MByte Flash*/
    asm volatile ("msr msp, %0"::"g"
                  (*(volatile uint32_t*)APP_LOAD_ADDRESS));
    /* Jump to application */
    (*(void(**)())(APP_LOAD_ADDRESS + 4))();
}
#endif

void bootloader(void)
{
    int state = BL_STATE_NONE, status;
    uint8_t buf[128];
    uint32_t data_addr = 0;
    data_buf_t dbuf;

    for (;;) {
        if (state == BL_STATE_NONE)
            led_off(LED_BL);
        else
            led_on(LED_BL);

        if (get_byte(buf, 100) == -1) {
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
            if (__read_data(buf + 1, 6) == -1)
                continue;

            /* TODO: Check that addr is within boundaries */
            if (crc8(buf, 6) != buf[6]) {
                status = BL_PROTO_STATUS_CRCERR;
            } else {
                data_addr = buf[1];
                status = BL_PROTO_STATUS_OK;
            }

            __send_status(status);
            break;
        case BL_PROTO_CMD_FLASH_DATA:
            bl_dbg("Flash data command.");
            if (__read_data(buf + 1, 3) == -1)
                continue;

            /* Checking CRC */
            if (crc8(buf, 3) != buf[3]) {
                status = BL_PROTO_STATUS_CRCERR;
            } else if (!buf[1] || (buf[1] + 1) % 4) { /* Thumb safe */
                status = BL_PROTO_STATUS_ARGERR;
            } else
                status = BL_PROTO_STATUS_OK;

            __send_status(status);

            if (status == BL_PROTO_STATUS_OK) {
                memset(&dbuf, 0, sizeof(dbuf));
                __read_data(dbuf.b, buf[1] + 1);
                /* TODO: Write data */
            }

            break;
        case BL_PROTO_CMD_DATA_CRC:
            bl_dbg("CRC check command.");
            /* TODO: Get CRC32 and send back */
            __send_data_crc();
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
    uint8_t msg[] = {
        BL_PROTO_VER, BL_PROTO_REV,
        board_id[0], board_id[1], board_id[2], board_id[3],
        0, BL_PROTO_EOM, 0};
    msg[8] = crc8(msg, 8);

    usb_msgsend(msg, 9);
}

static inline void __send_data_crc()
{
    uint32_t crc = 65536;
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

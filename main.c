// C source code header -*- coding: utf-8 -*-
// Created: [13.16:57 Январь 03 2014]
// Modified: [23.12:36 Март 03 2014]
// Description:
// Author: Stanislav M. Ivankin
// Email: lessgrep@gmail.com
// Tags: C,stm32
// License: GPLv2

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <errno.h>

#include <libopencm3/stm32/gpio.h>

#include <defs.h>
#include <board.h>
#include <init.h>

#include <mod/led.h>
#include <mod/usb.h>
#include <mod/usart.h>
#include <mod/spi.h>
#include <mod/at45db.h>
#include <mod/timer.h>

#include <fatfs/fatfs.h>

#include <bl.h>

static int flash_init(void);
static int flash_read(uint8_t *buf, off_t sector, unsigned int sector_num);

fatfs_t fs = {
    .hwops = {
        .init = &flash_init,
        .read = &flash_read,
    },
};

int test_fatfs(void)
{
    int ret;
    uint8_t buf[1024];

    memset(buf, 0, 1024);

    ret = fatfs_mount(&fs);
    if (ret != 0) {
        bl_dbg("Failed mounting flash device.");
    }

    ret = fatfs_open(&fs, "CONFIG.INI");
    if (ret) {
        bl_dbg("Failed opening config file.");
    }
    ret = fatfs_read(&fs, buf, 1024);
    if (ret < 0) {
        bl_dbg("Failed reading data.");
    }
    buf[ret] = '\0';
    d_print("%s\r\n", buf);

    fatfs_close(&fs);

    fatfs_umount(&fs);

    bl_dbg("Done");
#if 0
    int k;
    for (i = 44; i < 8192; i++) {
        at45db_bread(i, 1, buf);
        for (k = 0; k < 1024; k++) {
            d_print("%c", buf[k]);
            wait(25);
        }
        d_print("Page: %d\r\n", i);
        wait(5000);
    }
#endif
    return 0;
}

int main(void)
{
    bl_init();

    usart_start();
    spi_start();
    if (usbd_start()) {
        led_blink(LED_ACTIVITY, LED_STATE_RAPID);
        for (;;);
    }

    /* HW initialized */
    led_on(LED_ACTIVITY);
    bl_dbg("Bootloader started.");

//    test_fatfs();

    if (usb_connect()) {
        led_on(LED_USB);
        bl_dbg("USB connected.");
        bl_listen();
    } else
        led_off(LED_USB);

    bl_dbg("No USB.");

    jump_to_app(APP_LOAD_ADDRESS);

    return 0;
}

static int flash_init(void)
{
    return at45db_start();
}

static int flash_read(uint8_t *buf, off_t sector, unsigned int sector_num)
{
    int ret = 0;
    off_t off;

    off = (sector << SECTOR_PGSHIFT);

    d_print("Reading %d bytes at offset %lu\r\n",
            sector_num << SECTOR_PGSHIFT, off);

    ret = at45db_read(off, sector_num << SECTOR_PGSHIFT, buf);
    if (ret == -1)
        return -EIO;

    return sector_num;
}

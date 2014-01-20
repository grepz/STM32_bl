// C source code header -*- coding: utf-8 -*-
// Created: [13.16:57 Январь 03 2014]
// Modified: [19.10:23 Январь 20 2014]
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

#include "defs.h"
#include "board.h"
#include "init.h"

#include "mod/led.h"
#include "mod/usb.h"
#include "mod/usart.h"
#include "mod/spi.h"
#include "mod/at45db.h"

#include <fatfs/fatfs.h>

#include "bl.h"

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
    int i, ret;
    uint8_t buf[2048];

    memset(buf, 0, 2048);

    ret = fatfs_mount(&fs);
    if (ret != 0) {
        bl_dbg("Failed mounting flash device.");
    }

    ret = fatfs_open(&fs, "test.ini");
    if (ret) {
        bl_dbg("Failed opening config file.");
    }

    for (i = 0; i < 4096; i++) {
        ret = fatfs_read(&fs, buf, 100);
        if (ret < 0)
            for (;;);
        else if (!ret)
            break;
        buf[ret] = '\0';
        d_print("%s\r\n", buf);
    }

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
//    if (usbd_start()) {
//        led_blink(LED_ACTIVITY, LED_STATE_RAPID);
//        for (;;);
//    }

    /* HW initialized */
    led_on(LED_ACTIVITY);
    bl_dbg("Bootloader started.");

//    at45db_start();
//    at45db_chip_erase();
    test_fatfs();
    for (;;);

#if 0
    int i;
    for (;;) {
        for (i = 0; i < 1000000; i++)
            asm("nop");
        if (usb_connect()) {
            bl_dbg("1");
            led_on(LED_USB);
        } else {
            bl_dbg("0");
            led_off(LED_USB);
        }
    }
#endif

    if (usb_connect()) {
        led_on(LED_USB);
        bl_dbg("USB connected.");
        bootloader();
    } else {
        bl_dbg("No USB.");
        jump_to_app(APP_LOAD_ADDRESS);
    }

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

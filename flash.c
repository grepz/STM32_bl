#include <stdlib.h>
#include <stdint.h>

#include "defs.h"

#include "flash.h"

#define BOARD_FLASH_SIZE            (1024 * 1024)
#define BOOTLOADER_RESERVATION_SIZE (16 * 1024)
#define APP_SIZE_MAX (BOARD_FLASH_SIZE - BOOTLOADER_RESERVATION_SIZE)
#define FLASH_PROGRAM_X32 (0x02 << 8)

static const uint32_t __sector_size[] = {
    16*1024,  16*1024,  16*1024,  64*1024,  128*1024, 128*1024, 128*1024,
    128*1024, 128*1024, 128*1024, 128*1024, 16*1024,  16*1024,  16*1024,
    16*1024,  64*1024,  128*1024, 128*1024, 128*1024, 128*1024, 128*1024,
    128*1024, 128*1024,
};

inline uint32_t bl_flash_sector_size(uint32_t sector)
{
    if (sector < BL_FLASH_SECTORS)
        return __sector_size[sector];

    return 0;
}

uint32_t bl_flash_read_word(uint32_t address)
{
    if (address & 3)
        return 0;

    return *(uint32_t *)(address + APP_LOAD_ADDRESS);
}

int bl_flash_erase_sector(uint32_t sector)
{
//    uint8_t blank = 1;
    uint32_t size, i;
    uint32_t address = 0;

    if (sector >= BL_FLASH_SECTORS)
        return -1;

    for (i = 0; i < sector; i++)
        address += bl_flash_sector_size(i);

    size = bl_flash_sector_size(sector);
    for (i = 0; i < size; i += sizeof(uint32_t))
        if (*(uint32_t *)(address + i) != 0xffffffff) {
            flash_erase_sector(sector, FLASH_PROGRAM_X32);
            break;
        }

    return 0;
}

#include <stdlib.h>
#include <stdint.h>

#include "defs.h"

#include "flash.h"

#define FLASH_PROGRAM_X32 (0x02 << 8)

#if 0
static const uint32_t __sector_size[] = {
    16*1024, /* Bootloader page */
    16*1024,
    16*1024,
    64*1024,
    128*1024,
    128*1024,
    128*1024,
    128*1024,
    128*1024,
    128*1024,
    128*1024,
    16*1024,
    16*1024,
    16*1024,
    16*1024,
    64*1024,
    128*1024,
    128*1024,
    128*1024,
    128*1024,
    128*1024,
    128*1024,
    128*1024,
};
#endif

static const uint32_t __sector_size[] = {
    16*1024, /* Bootloader page */
    16*1024,
    16*1024,
    16*1024,
    16*1024,
    16*1024,
    16*1024,
    16*1024,
    16*1024,
    16*1024,
    16*1024,
    16*1024,
    16*1024,
    16*1024,
    16*1024,
    16*1024,
    16*1024,
    16*1024,
    16*1024,
    16*1024,
    16*1024,
    16*1024,
    16*1024,
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

    return *(uint32_t *)(address + STM32_BASE_ADDR);
}

void bl_flash_write_word(uint32_t address, uint32_t word)
{
    flash_program_word(address + STM32_BASE_ADDR, word);
}

int bl_flash_erase_sector(uint32_t sector)
{
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

int bl_flash_get_sector_num(uint32_t addr, uint32_t sz,
                            unsigned int *s, unsigned int *e)
{
    unsigned int i;
    size_t sector = 0;
    int start, end;

    start = end = -1;

    for (i = 0; i < sizeof(__sector_size); i++) {
        sector += __sector_size[i];
        if (addr < sector) {
            sector -= __sector_size[i];
            start = i;
            break;
        }
    }
    for (i = start; i < sizeof(__sector_size); i++) {
        sector += __sector_size[i];
        if ((addr + sz) < sector) {
            end = i;
            break;
        }
    }

    if (start == -1 || end == -1)
        return -1;

    *s = start;
    *e = end;

    return 0;
}

#ifndef __BL_FLASH_H
#define __BL_FLASH_H

#include <stdint.h>
#include <libopencm3/stm32/f4/flash.h>

typedef union {
    uint8_t  b[256];
    uint32_t w[64];
} data_buf_t;

inline uint32_t bl_flash_sector_size(uint32_t sector);
inline uint32_t bl_flash_read_word(uint32_t address);
int bl_flash_erase_sector(uint32_t sector);

#endif /* __BL_FLASH_H */

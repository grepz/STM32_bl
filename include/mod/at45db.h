#ifndef __BL_AT45DB_H
#define __BL_AT45DB_H

typedef struct __flash_storage
{
    int pg_num;
    int pg_shifts;
    uint32_t block_sz;
    uint32_t erase_sz;
    uint32_t n_eraseblocks;
} flash_storage_t;

int at45db_start(void);
void at45db_chip_erase(void);
ssize_t at45db_read(uint32_t off, size_t n, uint8_t *buf);
int at45db_erase(uint32_t sblock, size_t n);
ssize_t at45db_bread(uint32_t sblock, size_t n, uint8_t *buf);
ssize_t at45db_bwrite(uint32_t sblock, size_t n, const uint8_t *buf);

#endif /* __BL_AT45DB_H */

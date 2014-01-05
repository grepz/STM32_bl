#ifndef __BL_CONSOLE_H
#define __BL_CONSOLE_H

typedef struct __bl_console
{
    int (*write)(uint8_t *buf, size_t len);
    int (*read)(uint8_t *buf);
};

void console_print(const char *s, size_t n);

#endif /* __BL_CONSOLE_H */

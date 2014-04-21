#ifndef __BL_DEFS_H
#define __BL_DEFS_H

#include <mod/usart.h>

#define BL_TIMEOUT 15000

#ifdef BLDEBUG
#define bl_dbg(msg) print(msg"\r\n", strlen(msg) + 2)
#else
#define bl_dbg(msg)
#endif

#ifdef DPRINT
#define d_print(format, arg...)                   \
    do {                                          \
        char cc[256];                             \
        int ret;                                  \
        ret = snprintf(cc, 256, format, ##arg);   \
        print(cc, ret);                               \
    } while (0)
#else
#define d_print(x...)
#endif

#endif /* __BL_DEFS_H */

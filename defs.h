#ifndef __BL_DEFS_H
#define __BL_DEFS_H

#include "usart.h"

#define BL_TIMEOUT 15000

#ifdef BLDEBUG
#define bl_dbg(msg) print(msg"\r\n")
#else
#define bl_dbg(msg)
#endif

#ifdef DPRINT
#define d_print(format, arg...)                   \
    do {                                         \
        char cc[64];                             \
        snprintf(cc, 64, format, ##arg);          \
        print(cc);                               \
    } while (0)
#else
#define d_print(x...)
#endif

#endif /* __BL_DEFS_H */

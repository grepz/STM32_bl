#ifndef __BL_DEFS_H
#define __BL_DEFS_H

#include "usart.h"

#define BL_TIMEOUT 15000

#ifdef BLDEBUG
#define bl_dbg(msg) print(msg"\r\n")
#else
#define bl_dbg(msg)
#endif

#endif /* __BL_DEFS_H */

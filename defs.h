#ifndef __BL_DEFS_H
#define __BL_DEFS_H

#include "usart.h"

#define BL_TIMEOUT 5000

#define BL_PROTO_VER 0x1
#define BL_PROTO_REV 0x1

#ifdef BLDEBUG
#define bl_dbg(msg) print(msg"\r\n")
#else
#define bl_dbg(msg)
#endif

#endif /* __BL_DEFS_H */

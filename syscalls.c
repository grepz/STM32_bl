#include <stdint.h>
#include <stdlib.h>
#include <reent.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <defs.h>

#define UNUSED __attribute__((unused))

extern int  _end;

caddr_t _sbrk(int incr) {

    static unsigned char *heap = NULL;
    unsigned char *prev_heap;
    if (heap == NULL) {
        heap = (unsigned char *)&_end;
    }

    prev_heap = heap;
    heap += incr;

    return (caddr_t) prev_heap;
}

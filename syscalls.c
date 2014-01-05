#include <reent.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <stdint.h>

#include "defs.h"

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
#if 0

static char *heap_end;

void _exit(int UNUSED status)
{
    while(1) {;}
}

int _getpid(void)
{
    return 1;
}

int _kill(int pid, int sig)
{
    pid = pid; sig = sig;
//    errno = EINVAL;
    return -1;
}

int link(char UNUSED *old, char UNUSED *new) {
    return -1;
}

int _close(int file)
{
    return -1;
}

int _fstat(int UNUSED file, struct stat *st)
{
    st->st_mode = S_IFCHR;
    return 0;
}

int _isatty(int UNUSED file)
{
    return 1;
}

int _lseek(int UNUSED file, int UNUSED ptr, int UNUSED dir)
{
    return 0;
}

int _read(int UNUSED file, char UNUSED *ptr, int UNUSED len)
{
    return 0;
}

int _write(int UNUSED file, char UNUSED *ptr, int len)
{
    return len;
}

char* get_heap_end(void)
{
    return (char*) heap_end;
}

char* get_stack_top(void)
{
    return (char*) __get_MSP();
}
#endif

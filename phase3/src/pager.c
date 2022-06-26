#include "support/pager.h"
#include "os/const.h"
#include "support/memory.h"

#define SWAP_POOL_ADDR (KSEG1 + (32 * PAGESIZE))

// TODO: use another algorithm
static size_t i = -1;

inline size_t pick_page() { return (i = (i + 1) % POOLSIZE); }

inline memaddr page_addr(size_t i)
{
    if (i < 0 || i >= POOLSIZE)
        return (memaddr)NULL;
    return (memaddr)(SWAP_POOL_ADDR + i * PAGESIZE);
}

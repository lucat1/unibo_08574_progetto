#include "os/pcb.h"
#include "test/test.h"

int main()
{
    ensure("test1 returns 1") { assert(test1() == 1); }
}

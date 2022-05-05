/* Does nothing but outputs to the printer and terminates */

#include <umps/libumps.h>

#include "test/print.h"
#include "test/tconst.h"

void main()
{
    print(WRITETERMINAL, "printTest is ok\n");

    print(WRITETERMINAL, "Test Number 2 is ok\n");

    SYSCALL(TERMINATE, 0, 0, 0);
}

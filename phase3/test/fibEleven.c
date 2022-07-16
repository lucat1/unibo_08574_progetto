/*	Test of a CPU intensive recusive job */

#include <umps/libumps.h>

#include "test/print.h"
#include "test/tconst.h"

int fib(int i);

void main()
{
    int i;

    print(WRITETERMINAL, "Recursive Fibanaci (11) Test starts\n");

    i = fib(11);

    print(WRITETERMINAL, "Recursion Concluded\n");

    if (i == 89) {
        print(WRITETERMINAL, "Recursion Concluded Successfully\n");
    } else
        print(WRITETERMINAL, "ERROR: Recursion problems\n");

    /* Terminate normally */
    SYSCALL(TERMINATE, 0, 0, 0);
}

int fib(int i)
{
    if ((i == 1) || (i == 2))
        return (1);

    return (fib(i - 1) + fib(i - 2));
}
/*	Test the terminal for reading user input */

#include <umps/libumps.h>

#include "test/print.h"
#include "test/tconst.h"

void main()
{
    int status;
    char buf[15];

    print(WRITETERMINAL, "Terminal Read Test starts\n");
    print(WRITETERMINAL, "Enter a string: ");

    status = SYSCALL(READTERMINAL, (int)&buf[0], 0, 0);
    buf[status] = EOS;

    print(WRITETERMINAL, "\n");
    print(WRITETERMINAL, &buf[0]);

    print(WRITETERMINAL, "\n\nTerminal Read concluded\n");

    /* Terminate normally */
    SYSCALL(TERMINATE, 0, 0, 0);
}

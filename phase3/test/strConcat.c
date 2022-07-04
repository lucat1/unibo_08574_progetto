
/* concatenates two strings together and prints them out */

#include <umps/libumps.h>

#include "os/util.h"
#include "test/print.h"
#include "test/tconst.h"

void main()
{
    int status, status2, i;
    char buf[20];
    char buf2[20];
    char buf3[40];

    print(WRITETERMINAL, "Strcat Test starts\n");
    print(WRITETERMINAL, "Enter a string: ");

    status = SYSCALL(READTERMINAL, (int)&buf[0], 0, 0);
    buf[status] = EOS;

    print(WRITETERMINAL, "\n");
    print(WRITETERMINAL, "Enter another string: ");

    status2 = SYSCALL(READTERMINAL, (int)&buf2[0], 0, 0);
    buf2[status2] = EOS;

    i = 0;
    for (i = 0; i < status - 1; i++) {
        buf3[i] = buf[i];
    }

    for (i = 0; i < status2; i++) {
        buf3[status - 1 + i] = buf2[i];
    }

    buf3[status + status2 - 1] = EOS;

    print(WRITETERMINAL, &buf3[0]);

    print(WRITETERMINAL, "\n\nStrcat concluded\n");

    /* Terminate normally */
    SYSCALL(TERMINATE, 0, 0, 0);
}

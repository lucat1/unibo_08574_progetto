#include "os/scheduler.h"
#include "os/syscall.h"
#include "os/types.h"
#include "test/mock_init.h"
#include "test/mock_syscall.h"
#include "test/mock_iodev.h"
#include "test/test.h"
#include "os/const.h"
#include "os/util.h"
#include <stdio.h>
/* NSYS5 */
#define TERM0ADDR 0x10000254
#define PRINTCHR 2
#define IL_TERMINAL 7
#define RECVD 5
typedef unsigned int devregtr;

void print(char *msg)
{
    char *s = msg;
    devregtr *base = (devregtr *)(TERM0ADDR);
    devregtr *command = base + 3;
    devregtr status;
    devregtr value = PRINTCHR | (((devregtr)*s) << 8);
    printf("%c",*s);
    status = SYSCALL(DOIO, (size_t)command, (size_t)value, 0);
    if ((status & TERMSTATMASK) != RECVD) {
        printf("Panico");
        //PANIC();
    }
    s++;
}

int main()
{
    printf("anche prima");
    mock_init();
    active_process = spawn_process(false);
    printf("inizio\n");
    ensure("do_io blocks active_process on the correct semaphore")
    {
        print("test");
        //SYSCALL(DOIO, (size_t)command, (size_t)value, 0);
        assert(softblock_count == 1);
        assert(process_count == 1);
        assert(active_process->p_sem_add == get_semaphore(IL_TERMINAL, 0, true));
        
    }

    return 0;
}

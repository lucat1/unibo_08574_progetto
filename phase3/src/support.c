#include "support/support.h"
#include "os/scheduler.h"
#include "os/syscall.h"
#include "os/util.h"
#include <umps/cp0.h>

#define GETTOD 1
#define TERMINATE 2
#define WRITEPRINTER 3
#define WRITETERMINAL 4
#define READTERMINAL 5

// TODO
void support_tbl()
{

}

void support_trap()
{

}

void sys_get_tod()
{
    cpu_t time;
    STCK(time);
    active_process->p_s.reg_v0 = time;
}



void support_syscall()
{
    switch(active_process->p_s.reg_a0){
        case GETTOD:
            sys_get_tod();
            break;
        case TERMINATE:
            //syscall_terminate_process();
            break;
        case WRITEPRINTER:
            break;
        case WRITETERMINAL:
            break;
        case READTERMINAL:
            break;
        default:
            /*idk*/
            break;
    }
}


inline void support_generic()
{
    switch(CAUSE_GET_EXCCODE(active_process->p_support->sup_except_state->cause)){
        case 8: /*Syscall*/
            support_syscall();
            break;
        default:
            support_trap();
    }

}

#include "os/scheduler.h"
#include "os/syscall.h"
#include "os/types.h"
#include "test/mock_init.h"
#include "test/mock_syscall.h"
#include "test/mock_iodev.h"
#include "test/test.h"
#include "os/const.h"
#include "os/util.h"
#include <umps3/umps/cp0.h>
//#include <umps/libumps.h>

#define QPAGE 1024
#define CAUSEINTMASK 0xFD00


void set_state(state_t *new_process_state, memaddr fun){
    new_process_state->reg_sp = new_process_state->reg_sp - QPAGE;
    new_process_state->pc_epc = new_process_state->reg_t9 = fun;
    new_process_state->status = new_process_state->status | IEPON | CAUSEINTMASK | TEBITON;
}

void p1(){
    /* It is not supposed to do anything*/
}


int main()
{
    mock_init();
    active_process = spawn_process(false);
    pcb_t *p;
    state_t proc1;
    set_state(&proc1, (memaddr) p1);
    it("correctly creates a new high priority process")
    {
        assert(active_process->p_pid != -1);

        SYSCALL(CREATEPROCESS, (size_t)&proc1, true, 0);
        /* Assert the process gets allocated correctly */
        assert(active_process->p_s.reg_v0 != -1);

        /* Assert there is a new child of the active process */
        assert((p = remove_child(active_process)) != NULL);

        /* Assert there is at least one process in the high priority queue */
        assert(!list_empty(&ready_queue_hi));
    }
    it("correctly creates a new low priority process")
    {

        SYSCALL(CREATEPROCESS, (size_t)&proc1, true, 0);
        /* Assert the process gets allocated correctly */
        assert(active_process->p_s.reg_v0 != -1);

        /* Assert there is a new child of the active process */
        assert((p = remove_child(active_process)) != NULL);

        /* Assert there is at least one process in the low priority queue */
        assert(!list_empty(&ready_queue_lo));
    }
    it("correctly gets out of memory and returns -1"){
        /* There are already 3 processes allocated from previous code, so we need
         * To add at least 17 more processes to get out of memory
         */
        for(int i = 0; i<17; i++){
            SYSCALL(CREATEPROCESS, (size_t)&proc1, true, 0);
        }
        assert(active_process->p_s.reg_v0 != -1);
        SYSCALL(CREATEPROCESS, (size_t)&proc1, true, 0);
        assert(active_process->p_s.reg_v0 == -1);
    }
    return 0;
}

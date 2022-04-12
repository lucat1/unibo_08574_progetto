#include "os/const.h"
#include "os/pcb.h"
#include "os/scheduler.h"
#include "os/syscall.h"
#include "os/types.h"
#include "os/util.h"
#include "test/mock_init.h"
#include "test/mock_iodev.h"
#include "test/mock_syscall.h"
#include "test/test.h"
#include <stdio.h>
/* TESTING NSYS1 & NSYS2 */

void p1()
{ /* It is not supposed to do anything*/
}

int main()
{
    mock_init();
    active_process = spawn_process(false);
    state_t proc1;
    set_state(&proc1, (memaddr)p1);
    it("correctly creates a new high priority process")
    {
        assert(active_process->p_pid != NULL_PID);

        SYSCALL(CREATEPROCESS, (size_t)&proc1, true, 0);

        assert(active_process->p_s.reg_v0 != NULL_PID);
        assert(running_count == 2);
        assert(!list_empty(&active_process->p_child));
        assert(!list_empty(&ready_queue_hi));
    }
    it("correctly terminates the newly created process")
    {
        pcb_t *proc = find_process((pandos_pid_t)active_process->p_s.reg_v0);
        assert(proc->p_pid == active_process->p_s.reg_v0);
        SYSCALL(TERMPROCESS, proc->p_pid, 0, 0);
        assert(running_count == 1);
        assert(list_empty(&active_process->p_child));
        assert(list_empty(&ready_queue_hi));
    }
    it("correctly creates a new low priority process")
    {
        assert(active_process->p_pid != NULL_PID);

        SYSCALL(CREATEPROCESS, (size_t)&proc1, false, 0);

        assert(active_process->p_s.reg_v0 != NULL_PID);
        assert(running_count == 2);
        assert(!list_empty(&active_process->p_child));
        assert(!list_empty(&ready_queue_lo));
    }
    it("correctly terminates the newly created process")
    {
        pcb_t *proc = find_process((pandos_pid_t)active_process->p_s.reg_v0);
        assert(proc->p_pid == active_process->p_s.reg_v0);
        SYSCALL(TERMPROCESS, proc->p_pid, 0, 0);
        assert(running_count == 1);
        assert(list_empty(&active_process->p_child));
        assert(list_size(&ready_queue_lo) == 1);
    }
    it("correctly gets out of memory and returns NULL_PID")
    {
        /* There is already 1 process allocated from previous code, so we need
         * To add at least 19 more processes to get out of memory
         */
        for (int i = 0; i < 19; i++) {
            SYSCALL(CREATEPROCESS, (size_t)&proc1, true, 0);
        }
        assert(active_process->p_s.reg_v0 != NULL_PID);
        SYSCALL(CREATEPROCESS, (size_t)&proc1, true, 0);
        assert(active_process->p_s.reg_v0 == NULL_PID);
    }
    return 0;
}

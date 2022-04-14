#include "os/const.h"
#include "os/scheduler.h"
#include "os/syscall.h"
#include "os/types.h"
#include "os/util.h"
#include "test/mock_init.h"
#include "test/mock_iodev.h"
#include "test/mock_syscall.h"
#include "test/test.h"
/* NSYS8 */

void p1()
{
    return;
    /* It is not supposed to do anything*/
}

void trap_handler()
{
    return;
    /* It is not supposed to do anything*/
}

void memory_managment_handler()
{
    return;
    /* It is not supposed to do anything*/
}

int main()
{
    mock_init();
    active_process = spawn_process(false);
    state_t proc1;
    set_state(&proc1, (memaddr)p1);
    ensure("get_support_data returns null if the support structure is NULL")
    {
        SYSCALL(GETSUPPORTPTR, 0, 0, 0);
        assert((support_t *)active_process->p_s.reg_v0 == NULL);
    }
    ensure("get_support_data actually returns the active process support data")
    {
        support_t support_struct;
        unsigned int stack = proc1.reg_sp - QPAGE;
        support_struct.sup_except_context[GENERALEXCEPT].stack_ptr = (int)stack;
        support_struct.sup_except_context[GENERALEXCEPT].status =
            ALLOFF | IEPON | CAUSEINTMASK | TEBITON;
        support_struct.sup_except_context[GENERALEXCEPT].pc =
            (memaddr)trap_handler;
        support_struct.sup_except_context[PGFAULTEXCEPT].stack_ptr = stack;
        support_struct.sup_except_context[PGFAULTEXCEPT].status =
            ALLOFF | IEPON | CAUSEINTMASK | TEBITON;
        support_struct.sup_except_context[PGFAULTEXCEPT].pc =
            (memaddr)memory_managment_handler;

        active_process->p_support = &support_struct;

        SYSCALL(GETSUPPORTPTR, 0, 0, 0);
        assert((support_t *)active_process->p_s.reg_v0 == &support_struct);
        assert((support_t *)active_process->p_s.reg_v0 ==
               active_process->p_support);
    }

    return 0;
}

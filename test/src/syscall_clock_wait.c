/**
 * \file syscall_yield.c
 * \brief Tests around the CLOCKWAIT system call.
 *
 * \author Alessandro Frau
 * \date 12-04-2022
 */

#include "test/mock_init.h"
#include "test/mock_iodev.h"
#include "test/mock_syscall.h"
#include "test/test.h"

int main()
{
    mock_init();
    active_process = spawn_process(false);
    ensure(
        "wait_for_clock syscall blocks current process on the right semaphore")
    {
        SYSCALL(CLOCKWAIT, 0, 0, 0);
        assert(active_process->p_sem_add == get_timer_semaphore());
        assert(softblock_count == 1);
        assert(process_count == 1);
    }

    return 0;
}

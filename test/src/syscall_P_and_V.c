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
/* TESTING NSYS3 and NSYS4 */

void p1(){
    /* It is not supposed to do anything*/
}

int main()
{
    mock_init();
    active_process = spawn_process(false);
    state_t proc1;
    set_state(&proc1, (memaddr) p1);
    it("correctly P the current process on a semaphore")
    {
        assert(active_process->p_pid != -1);

        SYSCALL(PASSEREN, (size_t)&semaphores[0], 0, 0);
        assert(softblock_count == 1);
        assert(semaphores[0] == 0);
        assert(active_process->p_sem_add == &semaphores[0]);
        assert(process_count == 1);
    }
    it("correctly V the current process on it's semaphore")
    {
        SYSCALL(VERHOGEN, (size_t)&semaphores[0], 0, 0);
        printf("blocked %d, semaphore value: %d", softblock_count, semaphores[0]);
        assert(softblock_count == 0);
        assert(semaphores[0] == 0);
        assert(active_process->p_sem_add == NULL);
        assert(process_count == 1);
    }
    it("correctly P and V on every semaphores")
    {
        for(int i = 0; i<SEMAPHORES_NUM; i++){
            SYSCALL(PASSEREN, (size_t)&semaphores[i], 0, 0);
            assert(softblock_count == 1);
            assert(semaphores[i] == 0);
            assert(active_process->p_sem_add == &semaphores[i]);
            assert(process_count == 1);
            SYSCALL(VERHOGEN, (size_t)&semaphores[i], 0, 0);
            assert(softblock_count == 0);
            assert(semaphores[i] == 0);
            assert(active_process->p_sem_add == NULL);
            assert(process_count == 1);
        }
    }
    return 0;
}

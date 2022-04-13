#include "os/scheduler.h"
#include "os/syscall.h"
#include "os/types.h"
#include "test/mock_init.h"
#include "test/mock_syscall.h"
#include "test/mock_iodev.h"
#include "test/test.h"
#include "os/const.h"
#include "os/util.h"
/* NSYS6 */

int main()
{
    mock_init();
    active_process = spawn_process(false);
    ensure("get_cpu_time returns the correct value")
    {
        /* Custom cpu time for the sake of checking that the syscall
         * actually returns the value inside p_time
         */
        active_process->p_time = 100;
        SYSCALL(GETTIME, 0, 0, 0);
        assert(active_process->p_s.reg_v0 == active_process->p_time);
        assert(active_process->p_s.reg_v0 == 100);
    }

    return 0;
}

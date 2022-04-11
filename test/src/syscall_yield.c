#include "os/scheduler.h"
#include "os/syscall.h"
#include "os/types.h"
#include "test/mock_init.h"
#include "test/mock_iodev.h"
#include "test/mock_syscall.h"
#include "test/test.h"

int main()
{
    mock_init();
    it("updates yield_process")
    {
        active_process = spawn_process(false);
        assert(active_process->p_pid != -1);
        SYSCALL(YIELD, 0, 0, 0);
        assert(yield_process == active_process);
    }
    return 0;
}

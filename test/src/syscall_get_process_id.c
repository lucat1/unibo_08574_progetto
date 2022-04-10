#include "os/scheduler.h"
#include "test/mock_init.h"
#include "test/mock_iodev.h"
#include "test/mock_syscall.h"
#include "test/test.h"

int main()
{
    mock_init();
    it("sanitizes the input parameter")
    {
        active_process = spawn_process(false);
        assert(active_process->p_pid != -1);
        SYSCALL(GETPROCESSID, (size_t)NULL, 0, 0);
        assert(active_process->p_pid == -1);

        active_process = spawn_process(false);
        assert(active_process->p_pid != -1);
        SYSCALL(GETPROCESSID, -1, 0, 0);
        assert(active_process->p_pid == -1);

        active_process = spawn_process(false);
        assert(active_process->p_pid != -1);
        SYSCALL(GETPROCESSID, 2, 0, 0);
        assert(active_process->p_pid == -1);
    }
    return 0;
}

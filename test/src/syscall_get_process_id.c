#include "os/scheduler.h"
#include "test/mock_devices.h"
#include "test/mock_iodev.h"
#include "test/mock_processor.h"
#include "test/mock_syscall.h"
#include "test/test.h"

int main()
{
    it("sanitizes the input parameter")
    {
        active_process = spawn_process(false);
        SYSCALL(GETPROCESSID, (size_t)NULL, 0, 0);
        assert(active_process->p_pid == -1);

        active_process = spawn_process(false);
        SYSCALL(GETPROCESSID, -1, 0, 0);
        assert(active_process->p_pid == -1);

        active_process = spawn_process(false);
        SYSCALL(GETPROCESSID, 2, 0, 0);
        assert(active_process->p_pid == -1);
    }
    return 0;
}

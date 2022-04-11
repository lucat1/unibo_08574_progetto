#include "arch/devices.h"
#include "os/list.h"
#include "os/scheduler.h"
#include "test/mock_init.h"
#include "test/test.h"

int main()
{
    mock_init();
    ensure("the scheduler is initialized")
    {
        assert(!running_count);
        assert(!blocked_count);
        assert(list_empty(&ready_queue_hi));
        assert(list_empty(&ready_queue_lo));
        assert(active_process == NULL);
        assert(yield_process == NULL);
        assert(!get_recycle_count());
    }
    ensure("the semaphores are initialized")
    {
        for (int i = 0; i < 5 * DEVPERINT + 1; ++i) {
            assert(!semaphores[i]);
        }
    }
    ensure("the devices are reset")
    {
        assert(!interval_timer);
        assert(!local_timer);
        assert(!tod);
    }
    ensure("the processor is reset")
    {
        assert(!user_mode);
        assert(!halt_count);
        assert(!panic_count);
        assert(!wait_count);
        assert(!processor_state.cause);
        assert(!processor_state.status);
        assert(!processor_state.pc_epc);
        for (int i = 0; i < STATE_GPR_LEN; ++i)
            assert(!processor_state.gpr[i]);
    }
    return 0;
}

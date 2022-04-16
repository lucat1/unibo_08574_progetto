/**
 * \file init.c
 * \brief Tests concerning \ref init.c and \ref mock_init.h
 *
 * \author Stefano Volpe
 * \date 10-04-2022
 */

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
        assert(!process_count);
        assert(!softblock_count);
        assert(list_empty(&ready_queue_hi));
        assert(list_empty(&ready_queue_lo));
        assert(active_process == NULL);
        assert(yield_process == NULL);
        assert(!get_recycle_count());
    }
    /* These tests are rather a meta-test: we assert our testing infrastructure
     * is actually correct. */
    ensure("device semaphores are set to 0")
    {
        for (size_t i = 0; i < MOCK_SEMAPHORES_LEN; ++i)
            assert(semaphores[i] == 0);
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

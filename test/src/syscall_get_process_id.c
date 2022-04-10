#include "test/mock_processor.h"
#include "test/mock_puod.h"
#include "test/mock_syscall.h"
#include "test/test.h"

int main()
{
    active_process = alloc_pcb();
    it("sanitizes the input parameter")
    {
        reset_puod();
        SYSCALL(GETPROCESSID, (size_t)NULL, 0, 0);
        assert(puod_count == 2);

        reset_puod();
        SYSCALL(GETPROCESSID, -1, 0, 0);
        assert(puod_count == 1);

        reset_puod();
        SYSCALL(GETPROCESSID, 2, 0, 0);
        assert(puod_count == 1);
    }
    free_pcb(active_process);
    return 0;
}

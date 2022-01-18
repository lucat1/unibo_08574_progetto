#include "os/pcb.h"
#include "test/test.h"

int main()
{
    it("correctly initializes the list of PCBs") {
        state_t state = null_state();
        assert(state.status == UNINSTALLED);
    }
}

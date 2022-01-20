/** \file
 * \brief Tests concerning \ref pcb.h
 *
 * \author Alessandro Frau
 * \author Gianmaria Rovelli
 * \date 17-01-2022
 */

#include "os/pcb.h"
#include "test/test.h"

/// Runs the tests in question.
int main()
{
    it("correctly initializes the list of PCBs") {
        state_t state = null_state();
        assert(state.status == UNINSTALLED);
    }
}

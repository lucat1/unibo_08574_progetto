/**
 * \file syscall_get_support_data.c
 * \brief Tests around the GETSUPPORTPTR system call.
 *
 * \author Luca Tagliavini
 * \date 16-04-2022
 */

#include "test/mock_init.h"
#include "test/mock_iodev.h"
#include "test/mock_syscall.h"
#include "test/test.h"
#include <stdlib.h>
#include <time.h>

int main()
{
    srand(time(NULL));
    mock_init();
    active_process = spawn_process(false);
    ensure("get_support_data returns the address pointed of the pointed "
           "support_structure")
    {
        support_t *s = (support_t *)(size_t)rand();
        active_process->p_support = s;

        SYSCALL(GETSUPPORTPTR, 0, 0, 0);
        assert((support_t *)active_process->p_s.reg_v0 == s);
    }
    kill_progeny(active_process);

    return 0;
}

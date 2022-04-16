/**
 * \file
 * \brief Tests concerning \ref semaphores.h
 *
 * \author Stefano Volpe
 * \date 15-04-2022
 */

#include "os/semaphores.h"
#include "os/list.h"
#include "os/pcb.h"
#include "os/util.h"
#include "test/test.h"

int main()
{
    ensure("P(NULL, NULL) does not crash")
    {
        // P(NULL, NULL);
    }
    return 0;
}

/**
 * \file semaphores.c
 * \brief Tests concerning \ref semaphores.h
 *
 * \author Stefano Volpe
 * \date 15-04-2022
 */

#include "os/semaphores.h"
#include "test/mock_devices.h"
#include "test/mock_processor.h"
#include "test/test.h"

int main()
{
    ensure("P(NULL, NULL) does not crash") { P(NULL, NULL); }
    return 0;
}

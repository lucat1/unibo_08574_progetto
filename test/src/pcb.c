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
    ensure("test1 returns 1") { assert(test1() == 1); }
}

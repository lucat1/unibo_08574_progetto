/**
 * \file test.h
 * \brief Phase 3 test program and helper functions
 *
 * \author Luca Tagliavini
 * \author Stefano Volpe
 * \date 25-06-2022
 */

#ifndef PANDOS_TEST_H
#define PANDOS_TEST_H

#include "arch/processor.h"

/**
 * \brief Run the test program for the third fase.
 */
extern void test();

/**
 * \brief Free a Support Structure which was previously allocated by the test
 * program.
 * \param[in,out] s The Support Structure to be deallocated.
 */
extern void deallocate_support(support_t *s);

#endif /* PANDOS_TEST_H */

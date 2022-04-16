/**
 * \file test.h
 * \brief Support for a basic testing system
 *
 * \author Luca Tagliavini
 * \date 17-01-2022
 */

#ifndef PADOS_TEST_H
#define PADOS_TEST_H

#include <stdio.h>
#include <stdlib.h>

/* Name of the current test. */
char *test = NULL;

/** \brief Runs the following test with an "ensure" prefix in the message
 * printed on screen.
 * \param[in] name A string literal used as a short description for the test.
 * \see run
 */
#define ensure(name) run("ensure ", name)
/** \brief Runs the following test with an "it" prefix in the message printed on
 * screen.
 * \param[in] name A string literal used as a short description for the test.
 * \see run
 */
#define it(name) run("it ", name)
/** \brief Runs the following test.
 * on screen.
 * \param[in] prefix A string literal used as a prefix for the message printed
 *                   on screen.
 * \param[in] name   A string literal used as a short description for the test.
 */
#define run(prefix, name)                                                      \
    test = prefix name;                                                        \
    if (fprintf(stderr, "Running test: " prefix name "\n"))
/** \brief Halts the execution if the given expression is evaluated as 0.
 * \param[in] b The expression to evaluate.
 */
#define assert(b)                                                              \
    if (!(b)) {                                                                \
        fprintf(stderr, "Assertion failed: %s:%d (in '%s')\n", __FILE__,       \
                __LINE__, test);                                               \
        exit(1);                                                               \
    }

#endif /* PANDOS_TEST_H */

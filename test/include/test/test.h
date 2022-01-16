#ifndef PADOS_TEST_H
#define PADOS_TEST_H

#include <stdio.h>
#include <stdlib.h>

char *test = NULL;

#define ensure(name) run("ensure ", name)
#define it(name) run("it ", name)
#define run(prefix, name) test = prefix name; if(fprintf(stderr, "Running test: " prefix name "\n"))
#define assert(b) \
    if (!(b)) { \
        fprintf(stderr, "Assertion failed: %s:%d (in '%s')\n", __FILE__, __LINE__, test); \
        exit(1); \
    }

#endif /* PANDOS_TEST_H */

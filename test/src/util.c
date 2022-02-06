/** \file
 * \brief Tests concerning \ref util.h
 *
 * \author Luca Tagliavini
 * \author Stefano Volpe
 * \date 22-01-2022
 */

#include "os/util.h"
#include "test/test.h"
#include <string.h>

#define VALUE 1337

typedef struct data_t {
    int field;
    list_head l;
} data_t;

int cmp_fail(const list_head *first, const list_head *second)
{
    return container_of(second, data_t, l)->field != VALUE + 1;
}

int cmp(const list_head *first, const list_head *second)
{
    return container_of(second, data_t, l)->field != VALUE;
}

int main()
{
    const int len = 10;
    int i;
    data_t data[len];
    for (i = 0; i < len; ++i)
        data[i].field = 0;
    data[3].field = VALUE;
    LIST_HEAD(l);

    ensure("an empty list has size 0") { assert(!list_size(&l)); }

    for (i = 0; i < len; ++i)
        list_add(&data[i].l, &l);
    it("returns the corrent length for a list")
    {
        assert(list_size(&l) == len);
    }

    ensure("list_search returns NULL when no matches are found")
    {
        assert(!list_search(NULL, &l, cmp_fail));
    }
    ensure("list_search returns the correct head when it matches")
    {
        assert(list_search(NULL, &l, cmp));
    }
    ensure("itoa doesn't output when there isn't enough space") {
        char str[1];
        assert(!itoa(10, str, 1));
    }
    ensure("itoa produes the right output") {
        char str[3];
        assert(itoa(10, str, 3));
        assert(!strcmp(str, "10"));
        assert(itoa(-1, str, 3));
        assert(!strcmp(str, "-1"));
    }
    ensure("itoa trims the space to the right") {
        char str[100];
        assert(itoa(101, str, 100));
        assert(!strcmp(str, "101"));
    }
    return 0;
}

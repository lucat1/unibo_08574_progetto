/**
 * \file
 * \brief Tests concerning \ref util.h
 *
 * \author Luca Tagliavini
 * \author Stefano Volpe
 * \date 22-01-2022
 */

#include "os/util.h"
#include "os/util_impl.h"
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

    ensure("LIST_HEAD_NULL returns NULL")
    {
        assert(LIST_HEAD_NULL(&l) == NULL);
    }

    ensure("LIST_HEAD_NULL correctly updates its parameter")
    {
        assert(l.prev == NULL && l.next == NULL);
    }

    ensure("list_null correctly checks its parameter")
    {
        assert(list_null(&l));
        l.prev = l.next = &l;
        assert(!list_null(&l));
    }

    for (i = 0; i < len; ++i)
        list_add(&data[i].l, &l);
    it("returns the current length for a list")
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
    it("list_sdel safely deletes elements from a list")
    {
        for (i = 0; i < len; ++i) {
            list_sdel(&data[i].l);
            assert(list_size(&l) == len - 1 - i);
            assert(list_null(&data[i].l));
            list_sdel(&data[i].l);
            assert(list_size(&l) == len - 1 - i);
        }
    }
    ensure("str_writer respects the given length of a string")
    {
        char str[100];
        str_target_t w = {str, 100, 0};
        assert(str_writer((void *)&w, "t", 1) == 1);
        assert(str_writer((void *)&w, "es", 2) == 2);
        assert(str_writer((void *)&w, "t ", 2) == 2);
        assert(str_writer((void *)&w, " test ", 6) == 6);
        assert(str_writer((void *)&w, "test   ", 7) == 7);
        char *end = "";
        assert(!str_writer((void *)&w, end, 0));
    }
    ensure("itoa doesn't output when there isn't enough space")
    {
        char str[1];
        assert(!nitoa(10, 10, str, 1));
    }
    ensure("itoa returns the right amount of chars written")
    {
        char str[3];
        assert(nitoa(13, 10, str, 3) == 2);
    }
    ensure("itoa produces the right output")
    {
        char str[5];
        assert(nitoa(9, 10, str, 2));
        assert(!strcmp(str, "9"));
        assert(nitoa(-1, 10, str, 3));
        assert(!strcmp(str, "-1"));
        assert(nitoa(4, 2, str, 4));
        assert(!strcmp(str, "100"));
        assert(nitoa(64, 8, str, 4));
        assert(!strcmp(str, "100"));
        assert(nitoa(64, 16, str, 3));
        assert(!strcmp(str, "40"));
        assert(nitoa(28438, 16, str, 5));
        assert(!strcmp(str, "6f16"));
    }
    ensure("itoa trims the space to the right")
    {
        char str[100];
        assert(nitoa(101, 10, str, 100));
        assert(!strcmp(str, "101"));
    }
    ensure("__pandos_printf (snprintf) expands chars")
    {
        char str[10];
        pandos_snprintf(str, 10, "he%c%c%c", 'l', 'l', 'o');
        assert(!strcmp(str, "hello"));
    }
    ensure("__pandos_printf (snprintf) expands strings")
    {
        char str[10];
        pandos_snprintf(str, 10, "he%s", "llo");
        assert(!strcmp(str, "hello"));
    }
    ensure("__pandos_printf (snprintf) expands numbers")
    {
        char str[10];
        pandos_snprintf(str, 10, "%d%d", 13, 37);
        assert(!strcmp(str, "1337"));
    }
    ensure("snprintf trims at the given length")
    {
        char str[5];
        pandos_snprintf(str, 5, "Hello World");
        assert(!strcmp(str, "Hell"));
        pandos_snprintf(str, 5, "He%s World", "llo");
        assert(!strcmp(str, "Hell"));
        pandos_snprintf(str, 2, "%d%d", 13, 37);
        assert(!strcmp(str, "1"));
    }
    return 0;
}

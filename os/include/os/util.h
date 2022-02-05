/** \file
 * \brief Helper routines and such
 *
 * \author Luca Tagliavini
 * \author Stefano Volpe
 * \author Alessandro Frau
 * \date 17-01-2022
 */

#ifndef PANDOS_UTIL_H
#define PANDOS_UTIL_H

#include "list.h"
#include "types.h"
#ifdef PANDOS_TESTING
#include <stdio.h>
#endif

static inline int list_size(const list_head *head)
{
    int res;
    const list_head *iter;

    for (res = 0, iter = head->next; iter != head; ++res, iter = iter->next)
        ;

    return res;
}

static inline const list_head *
list_search(const list_head *element, const list_head *head,
            int (*cmp)(const list_head *, const list_head *))
{
    const list_head *iter;

    for (iter = head->next; iter != head; iter = iter->next)
        if (!cmp(element, iter))
            return iter;
    return NULL;
}

static inline int exact_cmp(const list_head *first, const list_head *second)
{
    return first != second;
}

static inline bool list_contains(const list_head *element,
                                 const list_head *head)
{
    return list_search(element, head, exact_cmp) != NULL;
}

static inline bool list_compare(const list_head *first, const list_head *second)
{
    return list_next(first) == list_next(second);
}

#ifdef PANDOS_TESTING
static inline void list_print(const list_head *head)
{
    const list_head *iter;
    if (head) {
        if (list_empty(head))
            printf("empty list\n");
        else
            for (iter = head->next; iter != head; iter = iter->next)
                printf("%p = {%p, %p}\n", iter, iter->prev, iter->next);
    } else
        printf("NULL list\n");
}
#endif

#endif /* PANDOS_UTIL_H */

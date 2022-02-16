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

/**
 * \brief Computes the size of a given list.
 * The sentinel element is not included in the size count.
 * \param[in] head The list whose size is to be computed.
 * \return The computed size.
 */
static inline int list_size(const list_head *head)
{
    int res;
    const list_head *iter;

    for (res = 0, iter = head->next; iter != head; ++res, iter = iter->next)
        ;

    return res;
}

/**
 * \brief Searches for an element in a list, using a given predicate to decide
 * whether two elements should be considered equal or not.
 * \param[in] element The element to be searched in the list.
 * \param[in] head The list to be searched in.
 * \param[in] cmp A pointer to a comparing function which returns 0 if and only
 * if its two parameters are to be considered equal.
 * \return A pointer to the first element which matches the query, or NULL if
 * such an element does not exist.
 */
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

/**
 * \brief Checks whether two list elements are the same.
 * \param[in] first The first list element.
 * \param[in] second The second list element.
 * \return Returns 0 if the two list elements are the same, or a non-zero value
 * if they are not.
 */
static inline int exact_cmp(const list_head *first, const list_head *second)
{
    return first != second;
}

/**
 * \brief Checks whether a certain list contains the given element or not.
 * \param[in] element The element to be searched.
 * \param[in] head The list where the element is searched.
 * \return Returns true if the element is found, or false otherwise.
 * if they are not.
 */
static inline bool list_contains(const list_head *element,
                                 const list_head *head)
{
    return list_search(element, head, exact_cmp) != NULL;
}


#ifdef PANDOS_TESTING
/**
 * \brief Prints the given list on standard output.
 * \param[in] head The list to be printed.
 */
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

/**
 * \brief 
 */
typedef struct str_writer {
    char *str;
    size_t size, wrote;
} str_writer_t;

size_t str_writer(void *dest, const char *data);
#endif

size_t nitoa(int i, int base, char *dest, size_t len);
size_t pandos_snprintf(char *dest, size_t len, const char *fmt, ...);

#ifndef __x86_64__
size_t pandos_fprintf(int fd, const char *fmt, ...);
#define pandos_printf(...) pandos_fprintf(0, __VA_ARGS__)
#endif

#endif /* PANDOS_UTIL_H */

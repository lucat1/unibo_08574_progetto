/**
 * \file util.h
 * \brief Helper routines and such
 *
 * \author Luca Tagliavini
 * \author Stefano Volpe
 * \author Alessandro Frau
 * \date 17-01-2022
 */

#ifndef PANDOS_OS_UTIL_H
#define PANDOS_OS_UTIL_H

#include "os/list.h"
#include "os/types.h"

/**
 * \brief Set both fields of a single list head to NULL.
 * \param[out] l The list head to be set to NULL.
 * \return Always NULL.
 */
#define NULL_LIST_HEAD(l) ((l).prev = (l).next = NULL)

/**
 * \brief Checks whether both fields of a list head are NULL.
 * \param[in] l The list head to be checked.
 * \return true if both fields are NULL, or false otherwise.
 */
#define IS_NULL_LIST_HEAD(l) ((l).prev == NULL && (l).next == NULL)

/**
 * \brief Deletes a single element from its list.
 * Features input validation and cleanup on success.
 * \param[in,out] entry The element to be removed.
 */
static inline void list_sdel(list_head *entry)
{
    if (!IS_NULL_LIST_HEAD(*entry)) {
        list_del(entry);
        NULL_LIST_HEAD(*entry);
    }
}

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

static inline void pandos_memcpy(void *dest, void *src, size_t len)
{
    char *s = (char *)src;
    char *d = (char *)dest;
    for (size_t i = 0; i < len; ++i)
        d[i] = s[i];
}

/**
 * \brief Computes the mathematical power of an integer base with a
 * non-negative integer as the exponent. The time cost of a single call
 * to this procedure is log(exp). \param[in] base The base of the power.
 * \param[in] exp The exponent of the power.
 * \return Returns the result of the exponentiation.
 */
int pandos_pow(int base, unsigned int exp);

/**
 * \brief Computes a string representation of a given integer.
 * \param[in] i The integer whose representation is to be computed.
 * \param[in] base The positional base of the representation.
 * \param[out] dest The string buffer where the result is to be stored.
 * \param[in] len The maximum length of the buffer which may be used.
 * \return The length of the string representation actually computed.
 */
size_t nitoa(int i, int base, char *dest, size_t len);

/**
 * \brief Prints formatted text on a string buffer up to a certain
 * number of characters. \param[out] dest The string buffer on which the
 * formatted text is to be printed. \param[in] len The maximum number of
 * characters to be printed. \param[in] fmt The format string to be
 * printed. \param[in] ... Additional parameters for the format string.
 * \return The number of characters actually printed.
 */
size_t pandos_snprintf(char *dest, size_t len, const char *fmt, ...);

#ifndef __x86_64__

/**
 * \brief Prints formatted text on a stream.
 * \param[out] fd The file descriptor for the stream to be used.
 * \param[in] fmt The format string to be printed.
 * \param[in] ... Additional parameters for the format string.
 * \return The number of characters actually printed.
 */
size_t pandos_fprintf(int fd, const char *fmt, ...);

/**
 * \brief Prints formatted text on standard output.
 * \param[in] fmt The format string to be printed.
 * \param[in] ... Additional parameters for the format string.
 * \return The number of characters actually printed.
 */
#define pandos_printf(fmt, ...) pandos_fprintf(0, fmt, ##__VA_ARGS__)

/* Number of lines in the memory print buffer */
#define MEM_WRITER_LINES 64
/* Length of a single line in characters */
#define MEM_WRITER_LINE_LENGTH 40

/**
 * \brief Utility structure for streaming data to raw memory.
 */
typedef struct memory_target {
    char buffer[MEM_WRITER_LINES][MEM_WRITER_LINE_LENGTH];
    size_t line; /** Index of the next to write */
    size_t ch;   /** Index of the current character in the current line */
} memory_target_t;

/* The variable to look for when debugging to check out the log */
extern memory_target_t kstdout, kstderr, kverb, kdebug;

size_t pandos_kfprintf(memory_target_t *, const char *fmt, ...);
#define pandos_kprintf(fmt, ...) pandos_kfprintf(&kstdout, fmt, ##__VA_ARGS__)
void pandos_kclear(memory_target_t *mem);
#else
#define pandos_kfprintf(...) ;
#define pandos_kprintf(...) ;
#endif

#ifndef __x86_64__
#define p pandos_kprintf
#else
#include <stdio.h>
#define p printf
#endif
/**
 * \brief Prints the given list on standard output.
 * \param[in] head The list to be printed.
 */
static inline void list_print(const list_head *head)
{
    const list_head *iter;
    if (head) {
        if (list_empty(head))
            p("empty list\n");
        else
            for (iter = head->next; iter != head; iter = iter->next)
                p("%p = {%p, %p}\n", (void *)iter, (void *)iter->prev,
                  (void *)iter->next);
    } else
        p("NULL list\n");
}
#undef p

#endif /* PANDOS_OS_UTIL_H */

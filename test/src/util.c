/** \file
 * \brief Tests concerning \ref util.h
 *
 * \author Luca Tagliavini
 * \author Stefano Volpe
 * \date 22-01-2022
 */

#include "os/util.h"
#include "test/test.h"

typedef struct data_t {
    list_head l;
} data_t;

int main()
{
    const int len = 10;
    int i;
    list_head list_heads[len];
    LIST_HEAD(l);

    ensure("an empty list has size 0")
    {
        assert(!list_size(&l));
    }
    
    for(i = 0; i < len; ++i)
        list_add(&list_heads[i], &l);
    it("returns the corrent length for a list")
    {
        assert(list_size(&l) == len);
    }
}


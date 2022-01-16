#include "os/asl.h"
#include "os/types.h"
#include "os/list.h"

static semd_t semd_table[MAX_PROC];
static struct list_head *semd_free;
static struct list_head *semd_h;

static int init_asl() 
{
    size_t i;

    INIT_LIST_HEAD(semd_free);
    INIT_LIST_HEAD(semd_h);
    for(i = 0; i < MAX_PROC; ++i) {
        semd_table[i].s_key = NULL;
        INIT_LIST_HEAD(&semd_table[i].s_procq);
        list_add(&(semd_table[i].s_link), semd_free);
    }
    return 0;
}

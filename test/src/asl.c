#include "os/asl.h"
#include "test/test.h"
#include <stdlib.h>
#include <time.h>

int main()
{
    it("intializes the table of semaphores correctly") {
        int i;
        semd_t *semd_table;

        semd_table = get_semd_table();
        init_asl();
        assert(!list_empty(get_semd_free()));
        assert(list_empty(get_semd_h()));
        for(i = 0; i < MAX_PROC; ++i) {
            /* Ensure the newly allocated semaphore is not in the busy list */
            assert(find_semd(get_semd_h(), semd_table[i].s_key) == NULL);
            /* Check that it belongs into the free list */
            assert(find_semd(get_semd_free(), semd_table[i].s_key) != NULL);
        }
    }
    srand(time(NULL));
    int key = rand();
    semd_t *created;
    ensure("insert_blocked creates a new semd when required") {
        assert(!insert_blocked(&key, NULL));
        created = find_semd(get_semd_h(), &key);
    }
    ensure("insert_blocked returns an existing semd when available") {
        assert(!insert_blocked(&key, NULL));
        assert(list_is_last(&created->s_link, get_semd_h()));
    }
}

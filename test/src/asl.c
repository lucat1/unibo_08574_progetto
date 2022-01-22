#include "os/asl.h"
#include "os/util.h"
#include "test/test.h"
#include <stdlib.h>
#include <time.h>

int main()
{
    int key;
    semd_t *created;
    pcb_t *example_pcb, *example_pcb1;

    srand(time(NULL));
    key = rand();
    init_asl();
    init_pcbs();
    example_pcb = allocPcb();
    example_pcb1 = allocPcb();
    /* alloc_semd */
    ensure("alloc_semd fails with a null argument") {
        assert(!alloc_semd(NULL));
    }
    it("correctly allocates a new semd") {
        semd_t *sem = alloc_semd(&key);
        assert(list_size(get_semd_h()) == 1);
        assert(list_size(get_semd_free()) == MAX_PROC - 1);
        assert(sem->s_key == &key);
        assert(list_empty(&sem->s_procq));
        free_semd(sem);
    }
    /* free_semd */
    ensure("free_smd fails with an illegal argument") {
        assert(free_semd(NULL));
        semd_t *sem = alloc_semd(&key);
        insert_blocked(&key, example_pcb);
        assert(free_semd(sem));
        remove_blocked(&key);
        assert(!free_semd(sem));
    }
    it("correctly deallocates an old semd") {
        semd_t *sem = alloc_semd(&key);
        assert(!free_semd(sem));
        assert(!list_size(get_semd_h()));
        assert(list_size(get_semd_free()) == MAX_PROC);
    }
    /* find_semd */
    ensure("find_semd fails with an illegal argument") {
        assert(!find_semd(NULL, &key));
        assert(!find_semd(get_semd_h(), NULL));
    }
    it("finds the desired semd in a list") {
        assert(!find_semd(get_semd_h(), &key));
        semd_t *semd = alloc_semd(&key);
        assert(find_semd(get_semd_h(), &key) == semd);
        free_semd(semd);
    }
    /* init_asl */
    it("intializes the table of semaphores correctly") {
        int i;
        semd_t *semd_table;

        semd_table = get_semd_table();
        assert(list_size(get_semd_free()) == MAX_PROC);
        assert(list_empty(get_semd_h()));
    }
    /* insert_blocked */
    ensure("insert_blocked fails with a null sem_addr or pcb") {
        assert(insert_blocked(NULL, example_pcb));
        assert(insert_blocked(&key, NULL));
    }
    example_pcb->p_semAdd = &key + rand();
    ensure("insert_blocked fails with a pcb that's already blocked") {
        assert(insert_blocked(&key, example_pcb));
    }
    example_pcb->p_semAdd = NULL;
    ensure("insert_blocked creates a new semd when required") {
        assert(example_pcb);
        assert(!insert_blocked(&key, example_pcb));
        created = find_semd(get_semd_h(), &key);
    }
    ensure("insert_blocked returns an existing semd when available") {
        assert(example_pcb);
        assert(!insert_blocked(&key, example_pcb1));
        assert(list_is_last(&created->s_link, get_semd_h()));
        printf("%d\n", list_size(&find_semd(get_semd_h(), &key)->s_procq));
    }
    ensure("insert_blocked is fair") {
        assert(remove_blocked(&key) == example_pcb);
        assert(remove_blocked(&key) == example_pcb1);
        printf("%d\n", list_size(&created->s_procq));
    }
    it("fails when there is no free space for semaphores") {
        list_head *backup_ptr = get_semd_free();
        const list_head backup = *backup_ptr;
        const list_head new = {backup_ptr, backup_ptr};
        set_semd_free(new);

        assert(insert_blocked(&key+MAX_PROC, example_pcb));

        set_semd_free(backup);
    }
    /* remove_blocked */
    ensure("remove_blocked fails with a wrong semaphore") {
        assert(!remove_blocked(NULL));
        assert(!remove_blocked(&key));
    }
    ensure("remove_blocked returns the right pcb") {
        assert(list_empty(get_semd_h()));

        assert(!insert_blocked(&key, example_pcb));
        assert(!insert_blocked(&key, example_pcb1));

        assert(remove_blocked(&key) == example_pcb);
        assert(list_size(&find_semd(get_semd_h(), &key)->s_procq) == 1);
        assert(remove_blocked(&key) == example_pcb1);

        assert(list_empty(get_semd_h()));
    }
    /* out_blocked */
    ensure("out_blocked fails with a wrong PCB") {
        assert(!out_blocked(NULL));
        
    }
    freePcb(example_pcb);
    freePcb(example_pcb1);
}

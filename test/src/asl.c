/**
 * \file asl.c
 * \brief Tests for the \ref asl.h library.
 *
 * \author Luca Tagliavini
 * \author Stefano Volpe
 * \date 15-01-2022
 */
#include "os/asl.h"
#include "os/asl_impl.h"
#include "os/util.h"
#include "test/mock_processor.h"
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
    example_pcb = alloc_pcb();
    example_pcb1 = alloc_pcb();
    /* alloc_semd */
    ensure("alloc_semd fails with a null argument")
    {
        assert(!alloc_semd(NULL));
    }
    it("correctly allocates a new semd")
    {
        semd_t *sem = alloc_semd(&key);
        assert(list_size(get_semd_h()) == 1);
        assert(list_size(get_semd_free()) == MAX_PROC - 1);
        assert(sem->s_key == &key);
        assert(list_empty(&sem->s_procq));
        free_semd(sem);
    }
    /* find_semd */
    ensure("find_semd fails with an illegal argument")
    {
        assert(!find_semd(NULL, &key));
        assert(!find_semd(get_semd_h(), NULL));
    }
    it("finds the desired semd in a list")
    {
        assert(!find_semd(get_semd_h(), &key));
        semd_t *semd = alloc_semd(&key);
        assert(find_semd(get_semd_h(), &key) == semd);
        free_semd(semd);
    }
    /* free_semd */
    ensure("free_smd fails with an illegal argument")
    {
        assert(free_semd(NULL));
        semd_t *sem = alloc_semd(&key);
        assert(!insert_blocked(&key, example_pcb));
        assert(free_semd(sem));
        remove_blocked(&key);
        assert(!free_semd(sem));
    }
    it("correctly deallocates an old semd")
    {
        semd_t *sem = alloc_semd(&key);
        assert(!free_semd(sem));
        assert(!list_size(get_semd_h()));
        assert(list_size(get_semd_free()) == MAX_PROC);
    }
    /* init_asl */
    it("initializes the table of semaphores correctly")
    {
        assert(list_size(get_semd_free()) == MAX_PROC);
        assert(list_empty(get_semd_h()));
    }
    /* insert_blocked */
    ensure("insert_blocked fails with a null sem_addr or pcb")
    {
        assert(insert_blocked(NULL, example_pcb));
        assert(insert_blocked(&key, NULL));
    }
    example_pcb->p_sem_add = &key + rand();
    ensure("insert_blocked fails with a pcb that's already blocked")
    {
        assert(insert_blocked(&key, example_pcb));
    }
    example_pcb->p_sem_add = NULL;
    ensure("insert_blocked creates a new semd when required")
    {
        assert(example_pcb != NULL);
        assert(!insert_blocked(&key, example_pcb));
        created = find_semd(get_semd_h(), &key);
    }
    ensure("insert_blocked actually appends the PCB to the semaphore list")
    {
        assert(list_size(&created->s_procq) == 1);
    }
    ensure("insert_blocked returns an existing semd when available")
    {
        assert(example_pcb != NULL);
        assert(!insert_blocked(&key, example_pcb1));
        assert(list_is_last(&created->s_link, get_semd_h()));
    }
    ensure("insert_blocked is fair")
    {
        assert(remove_blocked(&key) == example_pcb);
        assert(remove_blocked(&key) == example_pcb1);
    }
    it("fails when there is no free space for semaphores")
    {
        list_head *backup_ptr = get_semd_free();
        const list_head backup = *backup_ptr;
        const list_head new = {backup_ptr, backup_ptr};
        set_semd_free(new);

        assert(insert_blocked(&key + MAX_PROC, example_pcb));

        set_semd_free(backup);
    }
    /* out_blocked */
    ensure("out_blocked fails with a wrong PCB")
    {
        assert(out_blocked(NULL) == NULL);
        assert(out_blocked(example_pcb) == NULL);
        semd_t *semd = alloc_semd(&key);
        assert(!insert_blocked(&key, example_pcb));
        list_sdel(find_semd(get_semd_h(), &key)->s_procq.next);
        assert(out_blocked(example_pcb) == NULL);
        INIT_LIST_HEAD(&example_pcb->p_list);
        example_pcb->p_sem_add = NULL;
        free_semd(semd);
    }
    it("removes the appropriate PCB and returns it")
    {
        semd_t *semd = alloc_semd(&key);
        assert(!insert_blocked(&key, example_pcb));
        assert(!insert_blocked(&key, example_pcb1));
        assert(out_blocked(example_pcb) == example_pcb);
        assert(list_size(&semd->s_procq) == 1);
        assert(list_size(get_semd_h()) == 1);
        assert(out_blocked(example_pcb1) == example_pcb1);
        assert(!list_size(get_semd_h()));
        free_semd(semd);
    }
    /* head_blocked */
    ensure("head_blocked fails with a wrong sem_addr")
    {
        assert(head_blocked(NULL) == NULL);
        assert(list_empty(get_semd_h()));
        assert(head_blocked(&key) == NULL);
    }
    it("computes the head of a PCB queue without removing it")
    {
        semd_t *semd = alloc_semd(&key);
        assert(!insert_blocked(&key, example_pcb));
        assert(!insert_blocked(&key, example_pcb1));
        assert(head_blocked(&key) == example_pcb);
        assert(list_size(&semd->s_procq) == 2);
        assert(remove_blocked(&key) == example_pcb);
        assert(remove_blocked(&key) == example_pcb1);
    }
    /* remove_blocked */
    ensure("remove_blocked fails with a wrong semaphore")
    {
        assert(remove_blocked(NULL) == NULL);
        assert(remove_blocked(&key) == NULL);
    }
    ensure("remove_blocked returns the right pcb")
    {
        assert(list_empty(get_semd_h()));

        assert(!insert_blocked(&key, example_pcb));
        assert(!insert_blocked(&key, example_pcb1));

        assert(remove_blocked(&key) == example_pcb);
        assert(list_size(&find_semd(get_semd_h(), &key)->s_procq) == 1);
        assert(remove_blocked(&key) == example_pcb1);

        assert(list_empty(get_semd_h()));
    }
    free_pcb(example_pcb);
    free_pcb(example_pcb1);
    return 0;
}

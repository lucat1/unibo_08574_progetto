/**
 * \file scheduler.c
 * \brief Unit tests for the scheduler core.
 *
 * \author Luca Tagliavini
 * \date 11-04-2022
 */

#include "os/scheduler.h"
#include "os/asl_impl.h"
#include "os/const.h"
#include "os/pcb.h"
#include "os/scheduler_impl.h"
#include "os/types.h"
#include "test/mock_init.h"
#include "test/test.h"
#include <math.h>

int main()
{
    mock_init();
    /* The two following tests are to prevent regressions */
    ensure("enqueue_process does nothing with wrong input")
    {
        enqueue_process(NULL);
        assert(list_size(&ready_queue_hi) == 0);
        assert(list_size(&ready_queue_lo) == 0);
    }
    ensure("enqueue_process does not increment the running count")
    {
        size_t rc = process_count;
        pcb_t *p = alloc_pcb();
        enqueue_process(p);
        assert(process_count == rc);
        dequeue_process(p);
        free_pcb(p);
    }
    ensure("enqueue_process does not break a previous queue")
    {
        list_head head = LIST_HEAD_INIT(head);
        pcb_t *p = alloc_pcb();
        insert_proc_q(&head, p);
        assert(!list_null(&p->p_list));
        assert(!list_empty(&head));
        assert(list_contains(&p->p_list, &head));
        enqueue_process(p);
        assert(!list_null(&p->p_list));
        assert(list_empty(&head));
        assert(!list_contains(&p->p_list, &head));
        dequeue_process(p);
        free_pcb(p);
    }
    ensure("enqueue_process inserts the process in the appropriate queue")
    {
        pcb_t *p1 = alloc_pcb();
        pcb_t *p2 = alloc_pcb();
        p1->p_prio = true;
        p2->p_prio = false;
        enqueue_process(p1);
        enqueue_process(p2);
        assert(list_contains(&p1->p_list, &ready_queue_hi));
        assert(!list_contains(&p1->p_list, &ready_queue_lo));
        assert(list_contains(&p2->p_list, &ready_queue_lo));
        assert(!list_contains(&p2->p_list, &ready_queue_hi));
        dequeue_process(p1);
        dequeue_process(p2);
        free_pcb(p1);
        free_pcb(p2);
    }

    ensure("dequeue_process errors with wrong input")
    {
        assert(dequeue_process(NULL) == NULL);
        pcb_t *p = alloc_pcb();
        assert(list_null(&p->p_list));
        assert(dequeue_process(NULL) == NULL);
        assert(list_null(&p->p_list));

        /* The process is in a list but it's not a ready list */
        list_head head = LIST_HEAD_INIT(head);
        insert_proc_q(&head, p);
        assert(!list_null(&p->p_list));
        assert(!list_empty(&head));
        assert(list_contains(&p->p_list, &head));
        dequeue_process(p);
        assert(!list_null(&p->p_list));
        assert(!list_empty(&head));
        assert(list_contains(&p->p_list, &head));
        free_pcb(p);
    }
    ensure("dequeue_process correctly removes a process from its queue")
    {
        pcb_t *p1 = spawn_process(false);
        assert(process_count == 1);
        assert(!list_null(&p1->p_list));
        assert(list_contains(&p1->p_list, &ready_queue_lo));
        dequeue_process(p1);
        assert(list_empty(&ready_queue_lo));
        assert(!list_contains(&p1->p_list, &ready_queue_lo));
        kill_progeny(p1);

        pcb_t *p2 = spawn_process(true);
        assert(process_count == 1);
        assert(!list_null(&p2->p_list));
        assert(list_contains(&p2->p_list, &ready_queue_hi));
        dequeue_process(p2);
        assert(list_empty(&ready_queue_hi));
        assert(!list_contains(&p2->p_list, &ready_queue_hi));
        kill_progeny(p2);
    }

    ensure("spawn_process sanitizes the priority parameter")
    {
        size_t len = list_size(get_pcb_free());
        assert(spawn_process((bool)2) == NULL);
        assert(list_size(get_pcb_free()) == len);
        assert(spawn_process((bool)-1) == NULL);
        assert(list_size(get_pcb_free()) == len);
    }
    ensure("spawn_process behaves properly (except pid)")
    {
        size_t len = list_size(get_pcb_free()), count = process_count;
        for (int i = 0; i < 2; ++i) {
            pcb_t *p = spawn_process((bool)i);
            assert(p != NULL);
            assert(list_size(get_pcb_free()) == len - 1);
            assert(process_count == count + 1);
            assert(p->p_prio == (bool)i);
            assert(!list_null(&p->p_list));
            assert(!list_contains(get_pcb_free(), &p->p_list));
            assert(list_contains(i ? &ready_queue_hi : &ready_queue_lo,
                                 &p->p_list));
            kill_progeny(p);
        }
    }

    ensure("find_process returns NULL with invalid input")
    {
        assert(find_process(make_pid(-1, 0)) == NULL);
        assert(find_process(make_pid(MAX_PROC, 0)) == NULL);
        /* Valid format pid but pcb 8 is not alive */
        assert(find_process(make_pid(0, 0)) == NULL);
    }
    ensure("find_process the appropriate process")
    {
        pcb_t *p1 = spawn_process(false), *p2 = spawn_process(false);
        assert(find_process(p1->p_pid) == p1);
        assert(find_process(p1->p_pid) != p2);
        assert(find_process(p2->p_pid) == p2);
        assert(find_process(p2->p_pid) != p1);
        kill_progeny(p1);
        kill_progeny(p2);
    }

    ensure("the constant MAX_PROC_BITS is right")
    {
        assert(pow(2, MAX_PROC_BITS) >= MAX_PROC);
        assert(MAX_PROC_BITS < sizeof(pandos_pid_t) * BYTELENGTH);
    }
    ensure("spawn_process generates the right pid")
    {
        size_t recycle = get_recycle_count();
        int first_id =
            head_proc_q((list_head *)get_pcb_free()) - get_pcb_table();
        pcb_t *p1 = spawn_process(false);
        int second_id =
            head_proc_q((list_head *)get_pcb_free()) - get_pcb_table();
        pcb_t *p2 = spawn_process(false);
        assert(p1->p_pid != -1);
        assert(p2->p_pid != -1);
        assert(mask_pid_id(p1->p_pid) == first_id);
        assert(mask_recycle(p1->p_pid) == recycle);
        assert(mask_pid_id(p2->p_pid) == second_id);
        assert(mask_recycle(p2->p_pid) == recycle + 1);
        kill_progeny(p1);
        int third_id =
            head_proc_q((list_head *)get_pcb_free()) - get_pcb_table();
        p1 = spawn_process(false);
        assert(mask_pid_id(p1->p_pid) == third_id);
        assert(mask_recycle(p1->p_pid) == recycle + 2);
        kill_progeny(p1);
        kill_progeny(p2);
    }

    ensure("kill_process erros with broken parameters")
    {
        size_t count = process_count;
        assert(kill_process(NULL) == 1);
        assert(process_count == count);
        pcb_t *p = alloc_pcb(), *parent = alloc_pcb();
        p->p_parent = parent;
        assert(kill_process(p) == 2);
        assert(process_count == count);
        free_pcb(p);
        free_pcb(parent);
    }
    ensure("kill_process removes the process from a tree")
    {
        pcb_t *parent = spawn_process(false), *p = spawn_process(false);
        assert(parent->p_pid != NULL_PID);
        assert(p->p_pid != NULL_PID);
        insert_child(parent, p);
        assert(!empty_child(parent));
        assert(p->p_parent == parent);
        assert(kill_process(p) == 0);
        assert(empty_child(parent));
        assert(p->p_parent == NULL);
        assert(p->p_pid == NULL_PID);
        kill_progeny(parent);
    }
    ensure("kill_process removes the process from a semaphore")
    {
        int sem;
        pcb_t *p = spawn_process(false);
        assert(p->p_pid != NULL_PID);
        dequeue_process(p);
        insert_blocked(&sem, p);
        semd_t *semd = find_semd(get_semd_h(), &sem);
        assert(semd != NULL);
        assert(p->p_sem_add == &sem);
        assert(!list_empty(&p->p_list));
        assert(head_blocked(&sem) == p);
        assert(kill_process(p) == 0);
        assert(p->p_sem_add == NULL);
        assert(list_contains(get_semd_free(), &semd->s_link));
        assert(p->p_pid == NULL_PID);
        kill_progeny(p);
    }
    return 0;
}

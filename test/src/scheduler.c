/**
 * \file scheduler.c
 * \brief Unit tests for the scheduler core.
 *
 * \author Luca Tagliavini
 * \date 11-04-2022
 */

#include "os/scheduler.h"
#include "os/pcb.h"
#include "os/scheduler_impl.h"
#include "test/mock_init.h"
#include "test/test.h"
#include <math.h>

int main()
{
    mock_init();
    /* Preventing regressions */
    ensure("enqueue_process does nothing with wrong input")
    {
        size_t rc = process_count;
        enqueue_process(NULL);
        assert(process_count == rc);
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
    /* todo: loads of tests missing in between here, follow the c file */
    ensure("the constant MAX_PROC_BITS is right")
    {
        assert(MAX_PROC_BITS >= log(MAX_PROC) / log(2));
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
    return 0;
}

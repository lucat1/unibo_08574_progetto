/**
 * \file semaphores.c
 * \brief Tests concerning \ref semaphores.h
 *
 * \author Stefano Volpe
 * \date 15-04-2022
 */

#include "os/semaphores.h"
#include "os/asl.h"
#include "os/asl_impl.h"
#include "os/list.h"
#include "os/pcb.h"
#include "os/scheduler.h"
#include "os/util.h"
#include "test/mock_devices.h"
#include "test/mock_init.h"
#include "test/mock_processor.h"
#include "test/test.h"

static inline bool equals(scheduler_control_t sc1, scheduler_control_t sc2)
{
    return sc1.pcb == sc2.pcb && !sc1.enqueue == !sc2.enqueue;
}

int main()
{
    mock_init();
    ensure("P(NULL, NULL) works as expected")
    {
        assert(equals(P(NULL, NULL), CONTROL_RESCHEDULE));
    }
    ensure("P(NULL, &pcb) works as expected")
    {
        pcb_t pcb;
        LIST_HEAD_NULL(&pcb.p_list);
        assert(equals(P(NULL, &pcb), CONTROL_RESCHEDULE));
        assert(list_null(&pcb.p_list));
        assert(!softblock_count);
    }
    ensure("P(&s, NULL) works as expected")
    {
        int key = 0;
        semd_t *semd = alloc_semd(&key);
        assert(equals(P(&key, NULL), CONTROL_RESCHEDULE));
        assert(!key);
        assert(list_empty(&semd->s_procq));
        free_semd(semd);
    }
    ensure("P works fine with multiple semaphores active")
    {
        pcb_t pcb1, pcb2, pcb3;
        LIST_HEAD_NULL(&pcb1.p_list);
        LIST_HEAD_NULL(&pcb2.p_list);
        LIST_HEAD_NULL(&pcb3.p_list);
        int key = 0;
        semd_t *semd = alloc_semd(&key);

        assert(equals(P(&key, &pcb1), CONTROL_BLOCK));
        assert(!key);
        assert(list_size(&semd->s_procq) == 1);
        assert(softblock_count == 1);

        assert(equals(P(&key, &pcb2), CONTROL_BLOCK));
        assert(!key);
        assert(list_size(&semd->s_procq) == 2);
        assert(softblock_count == 2);

        assert(equals(P(&key, &pcb3), CONTROL_BLOCK));
        assert(!key);
        assert(list_size(&semd->s_procq) == 3);
        assert(softblock_count == 3);

        free_semd(semd);
        softblock_count = 0;
    }
    ensure("V(NULL) works as expected") { assert(V(NULL) == NULL); }
    ensure("V works fine with multiple semaphores active")
    {
        pcb_t pcb1, pcb2, pcb3;
        LIST_HEAD_NULL(&pcb1.p_list);
        pcb1.p_sem_add = NULL;
        LIST_HEAD_NULL(&pcb2.p_list);
        pcb2.p_sem_add = NULL;
        LIST_HEAD_NULL(&pcb3.p_list);
        pcb3.p_sem_add = NULL;
        int key = 0;
        semd_t *semd = alloc_semd(&key);

        active_process = &pcb1;
        assert(V(&key) == active_process);
        assert(key == 1);
        assert(list_empty(&semd->s_procq));
        assert(!softblock_count);

        active_process = &pcb2;
        assert(V(&key) == NULL);
        assert(key == 1);
        assert(list_size(&semd->s_procq) == 1);
        assert(softblock_count == 1);

        active_process = &pcb3;
        assert(V(&key) == NULL);
        assert(key == 1);
        assert(list_size(&semd->s_procq) == 2);
        assert(softblock_count == 2);

        free_semd(semd);
        active_process = NULL;
        softblock_count = 0;
    }
    ensure("P and V work fine together")
    {
        pcb_t pcb1, pcb2, pcb3;
        LIST_HEAD_NULL(&pcb1.p_list);
        pcb1.p_sem_add = NULL;
        LIST_HEAD_NULL(&pcb2.p_list);
        pcb2.p_sem_add = NULL;
        LIST_HEAD_NULL(&pcb3.p_list);
        pcb3.p_sem_add = NULL;
        int key = 0;
        semd_t *semd = alloc_semd(&key);

        assert(equals(P(&key, &pcb1), CONTROL_BLOCK));
        assert(!key);
        assert(list_size(&semd->s_procq) == 1);
        assert(softblock_count == 1);

        active_process = &pcb2;
        assert(V(&key) == &pcb1);
        assert(key == 0);
        assert(list_empty(&semd->s_procq));
        assert(!softblock_count);

        active_process = &pcb3;
        assert(V(&key) == &pcb3);
        assert(key == 1);
        assert(list_empty(&semd->s_procq));
        assert(!softblock_count);

        active_process = &pcb1;
        assert(V(&key) == NULL);
        assert(key == 1);
        assert(list_size(&semd->s_procq) == 1);
        assert(softblock_count == 1);

        assert(equals(P(&key, &pcb2), CONTROL_RESCHEDULE));
        assert(key == 1);
        assert(list_empty(&semd->s_procq));
        assert(!softblock_count);

        assert(equals(P(&key, &pcb3), CONTROL_RESCHEDULE));
        assert(!key);
        assert(list_empty(&semd->s_procq));
        assert(!softblock_count);

        free_semd(semd);
        active_process = NULL;
        softblock_count = 0;
    }
    return 0;
}

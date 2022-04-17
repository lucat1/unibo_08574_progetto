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
        pcb_t *pcbs[3];
        for (size_t i = 0; i < 3; ++i)
            pcbs[i] = alloc_pcb();
        int key = 0;
        semd_t *semd = alloc_semd(&key);

        assert(equals(P(&key, pcbs[0]), CONTROL_BLOCK));
        assert(!key);
        assert(list_size(&semd->s_procq) == 1);
        assert(softblock_count == 1);

        assert(equals(P(&key, pcbs[1]), CONTROL_BLOCK));
        assert(!key);
        assert(list_size(&semd->s_procq) == 2);
        assert(softblock_count == 2);

        assert(equals(P(&key, pcbs[2]), CONTROL_BLOCK));
        assert(!key);
        assert(list_size(&semd->s_procq) == 3);
        assert(softblock_count == 3);

        free_semd(semd);
        for (size_t i = 0; i < 3; ++i)
            free_pcb(pcbs[i]);
        softblock_count = 0;
    }
    ensure("V(NULL) works as expected") { assert(V(NULL) == NULL); }
    ensure("V works fine with multiple semaphores active")
    {
        pcb_t *pcbs[3];
        for (size_t i = 0; i < 3; ++i)
            pcbs[i] = alloc_pcb();
        int key = 0;
        semd_t *semd = alloc_semd(&key);

        active_process = pcbs[0];
        assert(V(&key) == active_process);
        assert(key == 1);
        assert(list_empty(&semd->s_procq));
        assert(!softblock_count);

        active_process = pcbs[1];
        assert(V(&key) == NULL);
        assert(key == 1);
        assert(list_size(&semd->s_procq) == 1);
        assert(softblock_count == 1);

        active_process = pcbs[2];
        assert(V(&key) == NULL);
        assert(key == 1);
        assert(list_size(&semd->s_procq) == 2);
        assert(softblock_count == 2);

        for (size_t i = 0; i < 3; ++i)
            free_pcb(pcbs[i]);
        free_semd(semd);
        active_process = NULL;
        softblock_count = 0;
    }
    ensure("P and V work fine together")
    {
        pcb_t *pcbs[3];
        for (size_t i = 0; i < 3; ++i)
            pcbs[i] = alloc_pcb();
        int key = 0;

        assert(equals(P(&key, pcbs[0]), CONTROL_BLOCK));
        assert(!key);
        assert(list_size(&find_semd(get_semd_h(), &key)->s_procq) == 1);
        assert(softblock_count == 1);

        active_process = pcbs[1];
        assert(V(&key) == pcbs[0]);
        assert(key == 0);
        assert(find_semd(get_semd_h(), &key) == NULL);
        assert(active_process->p_sem_add == NULL);
        assert(!softblock_count);

        active_process = pcbs[2];
        assert(V(&key) == pcbs[2]);
        assert(key == 1);
        assert(find_semd(get_semd_h(), &key) == NULL);
        assert(active_process->p_sem_add == NULL);
        assert(!softblock_count);

        semd_t *sem;
        active_process = pcbs[0];
        assert(V(&key) == NULL);
        assert(key == 1);
        assert((sem = find_semd(get_semd_h(), &key)) != NULL);
        assert(list_size(&sem->s_procq) == 1);
        assert(active_process->p_sem_add == &key);
        assert(softblock_count == 1);
        sem = NULL; /* Added safety */

        assert(equals(P(&key, pcbs[1]), CONTROL_RESCHEDULE));
        assert(key == 1);
        assert(find_semd(get_semd_h(), &key) == NULL);
        assert(pcbs[1]->p_sem_add == NULL);
        assert(!softblock_count);

        assert(equals(P(&key, pcbs[2]), CONTROL_RESCHEDULE));
        assert(!key);
        assert(find_semd(get_semd_h(), &key) == NULL);
        assert(pcbs[2]->p_sem_add == NULL);
        assert(!softblock_count);

        active_process = NULL;
        softblock_count = 0;
        for (size_t i = 0; i < 3; ++i)
            free_pcb(pcbs[i]);
    }
    return 0;
}

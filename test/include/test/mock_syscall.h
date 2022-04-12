/**
 * \file mock_syscall.h
 * \brief A mock of the BIOS SYSCALL procedure.
 *
 * \author Luca Tagliavini
 * \date 9-04-2022
 */

#ifndef PANDOS_TEST_MOCK_SYSCALL_H
#define PANDOS_TEST_MOCK_SYSCALL_H

#include "os/pcb.h"
#include "os/syscall.h"

#define QPAGE 1024
#define CAUSEINTMASK 0xFD00

size_t SYSCALL(size_t a0, size_t a1, size_t a2, size_t a3)
{
    active_process->p_s.reg_a0 = a0;
    active_process->p_s.reg_a1 = a1;
    active_process->p_s.reg_a2 = a2;
    active_process->p_s.reg_a3 = a3;
    syscall_handler();
    return active_process->p_s.reg_v0;
}

void set_state(state_t *new_process_state, memaddr fun){
    new_process_state->reg_sp = new_process_state->reg_sp - QPAGE;
    new_process_state->pc_epc = new_process_state->reg_t9 = fun;
    new_process_state->status = new_process_state->status | IEPON | CAUSEINTMASK | TEBITON;
}

#endif /* PANDOS_TEST_MOCK_SYSCALL_H */

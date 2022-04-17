/**
 * \file mock_devices.h
 * \brief A mock of architecture-dependant device utilities.
 *
 * \author Luca Tagliavini
 * \date 10-04-2022
 */

#ifndef PANDOS_TEST_MOCK_DEVICES_H
#define PANDOS_TEST_MOCK_DEVICES_H

#include "os/ctypes.h"

#define MOCK_IL_MASK_ALL 0x00FFFF00 /* bits 5-27 active */

int interval_timer;
int local_timer;
int tod;

void status_il_on_all(size_t *prev) { *prev |= MOCK_IL_MASK_ALL; }
void status_il_on(size_t *prev, int line) { *prev |= (1 << line); }

void store_tod(int *time) { *time = tod; }
void load_interval_timer(int time) { interval_timer = time; }
void load_local_timer(int time) { local_timer = time; }

void reset_devices()
{
    interval_timer = 0;
    local_timer = 0;
    tod = 0;
}

#endif /* PANDOS_TEST_MOCK_DEVICES_H */

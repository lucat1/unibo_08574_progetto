/**
 * \file semaphores_impl.h
 * \brief Implementation details for device semaphores.
 *
 * \author Gianmaria Rovelli
 * \author Luca Tagliavini
 * \date 13-04-2022
 */

#ifndef PANDOS_SEMAPHORE_IMPL_H
#define PANDOS_SEMAPHORE_IMPL_H

#define SEMAPHORES_NUM (DEVINTNUM + 1) * DEVPERINT + 1

/* Semaphores for each device */
extern int semaphores[SEMAPHORES_NUM];

#endif /* PANDOS_SEMAPHORE_IMPL_H */

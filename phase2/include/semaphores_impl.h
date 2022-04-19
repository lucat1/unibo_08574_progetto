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

/**
 * \brief Provides a pointer to the first element of the semaphores array.
 * \return A pointer to the first element of the semaphores array.
 */
extern int *get_semaphores();

#endif /* PANDOS_SEMAPHORE_IMPL_H */

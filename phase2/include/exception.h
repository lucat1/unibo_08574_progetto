/**
 * \file exception.h
 * \brief Exception handlers implementations.
 *
 * \author Alessandro Frau
 * \author Luca Tagliavini
 * \date 30-03-2022
 *
 */

#ifndef PANDOS_EXCEPTION_H
#define PANDOS_EXCEPTION_H

/**
 * \brief The handler called by the processor when an exception is raised. Calls
 * the appropriate handler (be it syscall, interrupt or trap) based on the value
 * read from the CAUSE register. After the handling of the exception the
 * scheduler is called explicitly to re-launch the appropriate next process.
 */
extern void exception_handler();

#endif /* PANDOS_EXCEPTION_H */

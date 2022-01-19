/** \file
 * \brief Type definitions
 *
 * \author Luca Tagliavini
 * \date 17-01-2022
 */

#ifndef PANDOS_TYPES_H
#define PANDOS_TYPES_H

/** Device register type for terminals */
typedef struct {
    /** Receiver status */
    unsigned int recv_status;
    /** Receiver command */
    unsigned int recv_command;
    /** Transmitter status */
    unsigned int transm_status;
    /** Transmitter command */
    unsigned int transm_command;
} termreg_t;

#endif /* PANDOS_TYPES_H */

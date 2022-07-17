/**
 * \file storage.h
 * \brief Support level flash devices drive
 *
 * \author Gianmaria Rovelli
 * \author Luca Tagliavini
 * \author Stefano Volpe
 * \date 25-06-2022
 */

#ifndef PANDOS_STORAGE_H
#define PANDOS_STORAGE_H

#include "os/ctypes.h"
#include "os/types.h"

/**
 * \brief Read the specified block from a given flash device.
 * \param[in] dev The flash device identifier.
 * \param[in] block The block 0-based index.
 * \param[out] dest The starting address to which save the block contents.
 * \return The device status after the reading attempt.
 */
extern int read_flash(unsigned int dev, size_t block, void *dest);

/**
 * \brief Write on the specified block from a given flash device.
 * \param[in] dev The flash device identifier.
 * \param[in] block The block 0-based index.
 * \param[out] src The starting address whose contents will be written.
 * \return The device status after the writing attempt.
 */
extern int write_flash(unsigned int dev, size_t block, void *src);

#endif /* PANDOS_STORAGE_H */

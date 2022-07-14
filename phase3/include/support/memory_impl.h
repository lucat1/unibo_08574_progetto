#ifndef PANDOS_MEMORY_IMPL_H
#define PANDOS_MEMORY_IMPL_H

#include "os/const.h"
#include "os/types.h"
#include "os/util.h"

typedef swap_t swap_table_t[MAXPAGES];

inline void set_swap_pool_baton(int asid, bool value);
inline bool get_swap_pool_baton(int asid);

#endif /* PANDOS_MEMORY_IMPL_H */

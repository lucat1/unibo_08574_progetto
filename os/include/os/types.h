#ifndef PANDOS_TYPES_H
#define PANDOS_TYPES_H

/* Device register type for terminals */
typedef struct {
	unsigned int recv_status;
	unsigned int recv_command;
	unsigned int transm_status;
	unsigned int transm_command;
} termreg_t;

#endif /* PANDOS_TYPES_H */

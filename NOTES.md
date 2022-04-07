# Additional notes

Procedures across the implementation and the provided test file have been
renamed to follow our naming convention. The include paths in the given header
and source files have also been adapted to our project structure.

Procedure `head_proc_q()` from `os/include/pcb.h` returns a pointer to a
`pcb_t`, despite the provided API mistakenly specifying `pcb_t` as the return
type.

Type alias `pandos_pid_t` from `os/include/os/types.h` is defined as an
`unsigned int`, despite the provided API suggesting each PIDs should be
represented as an `int`.

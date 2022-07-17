# Pandos+

## Overview

The PandOS+ project structure heavily relies on cmake sub-directory
functionality, which allows for modular compilation based on target architecture
and toggle flags. The root directory contains three major kind of
sub-directories:

- `os`: the major library providing all functionality required for the various
  phases and ultimately the proper operating system implementation. The library
  also contains utility procedures to aid debugging on the target machine;
- `test`: unit tests to be executed on the host's machine aimed at verifying the
  logic correctness of the functionality implemented in the `os` sub-project.
  These unit tests should complement the provided `phase{n}` tests;
- `phase{n}`: the code for each phase of the project to be compiled into a MIPS
  kernel and tested on the uMPS3 emulator.

## Design choices

The identifier for a new born process (pid for short) is generated in a
semi-unique way, which provides a good balance between raw performance and
robustness. The 32 bits of the pid are initially subdivided into two sections:

- the lower `MAX_PROC_BITS` bits identify the index of the underlying `pcb_t`
  structure in the table of all `pcb_t`s;
- the remaining `WORD_BITS - MAX_PROC_BITS` bits are set to the lower bits of
  the `recycle_count` counter.

Finally, the resulting pid is incremented by one. This is done to reserve 0 as
the value of the conventional `NULL_PID`, which does not describe any valid pid
at all. The value of `MAX_PROC_BITS` is computed by hand based on the value of
`MAX_PROC`, preserving the following invariant:

```math
MAX\_PROC\_BITS \geq \lceil log_2(MAX\_PROC) \rceil
```

This approach does not guarantee a complete uniqueness (which cannot be achieved
with any limited word length) but is nonetheless an improvement over the simpler
usage of indices/addresses. Compared to incremental pids the chosen implementation
lowers the address space significantly by halving possible pids by a factor of
`2^MAX_PROC_BITS` and using a shared recycle count for all processes. An
improvement (at the cost of some memory) could use an array of `MAX_PROC`
recycle counters, having one for each process. This latter approach could also
improve safety by making pid spoofing much harder.

## Building

This project uses `cmake` and `make` to compile and requires the host's C
compiler along with the MIPSel cross compilation toolchain even when targeting
the host machine. The following cmake options are available:

- `CROSS_COMPILE`: defaults to `ON (1)`. Toggle the target of compilation
  between the host's system (`OFF (0)`) and the MIPS architecture (`ON (1)`);
- `DEBUG`: defaults to `OFF (0)`. Whether to attach gdb debug information to the
  compiled object, improving the debugging experience on the host machine;
- `CMAKE_EXPORT_COMPILE_COMMANDS`: defaults to `OFF (0)`. Can be set to true to
  generate a `compile_commands.json` in the artifacts directory containing all
  compile flags for each file of the project. It's required by language servers
  like `clangd` or `ccls` to offer meaningful advice.

### Cross-compilation for the MIPS architecture:

The standard default cmake build will compile all project phases (i.e. the tests
in `phase{n}`) and generate a UMPS3 kernel for each. Provided you call the
artifact directory `build` the emulator configuration found in
`machines/phase{n}` should work out of the box.

```sh
mkdir build && cd build
cmake ..
make
```

If you have any issues with `machines/phase3`, please try using
`machines/phase3-bin` instead.

### Compiling on the host machine for testing

The cmake build system allows us to easily swap C compilers and we can use that
to our advantage. Compiling for the host's system architecture allows us to run
the contents of the `test` directory locally and check for logic correctness. We
can therefore disable cross-compilation via the `CROSS_COMPILE` option and run
unit tests as shown below:

```sh
mkdir buildt && cd buildt
cmake -DCROSS_COMPILE=0 ..
make && make test
```

### Documentation

The project offline documentation can be generated via
[Doxygen](https://www.doxygen.nl). The following commands will produce the
output TeX files in `build/docs/latex`.

```sh
mkdir build && cd build
cmake ..
make docs
```

### Setup workspace with the Clangd LSP

Any C Language Server needs to know how all source files will be compiled
(namely the flags given to the compiler) to correctly work. Providing the flags
for each file can be tedious, so using the cmake builtin functionality is
preferred:

```sh
mkdir build && cd build
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 ..
mv compile_commands.json ..
```

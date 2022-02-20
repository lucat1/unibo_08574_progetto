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

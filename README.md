# Building

## Cross compilation for the MIPS (particular focus on UMPS3) architecture:
The standard default cmake build will compile all project phases and generate a
UMPS3 kernel for each. Calling the artifact directory `build` allows the
emulator configuration found in `machines/phase{n}` to work out of the box.
```sh
mkdir build && cd build
cmake ..
make
```

## Documentation
The project offline documentation is also available via cmake in Latex format.
The following commands will output it in `build/docs/latex/`.
```sh
mkdir build && cd build
cmake ..
make docs
```

## Native compilation for testing
The cmake build system allows us to easily swap C compilers and we can use that
to our advantage. Compiling for the host's system architecture allows to run
unit testing locally and check the code for logic correctness. We can therefore
disable `CROSS_COMPILE` and run tests as shown below:
```sh
mkdir buildt && cd buildt
cmake -DCROSS_COMPILE=0 ..
make && make test
```

## Setup workspace with the Clangd LSP
In order for clangd to understand what your workspace looks like it has to know
which compile flags are used to compile each file. Setting up this by hand can
be tedious, but thankfully `cmake` can do that for us:
```sh
mkdir build && cd build
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 ..
mv compile_commands.json ..
```

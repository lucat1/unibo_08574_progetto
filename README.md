# Building

## Cross compilation for the MIPS (particular focus on UMPS3) architecture:
The standard default cmake build will compile all project phases and generate a
UMPS3 kernel for each. Calling the artifact directory `build` allows the emulator
configuration found in `machines/phase{n}` to work out of the box.
```sh
mkdir build && cd build
cmake ..
make
```

## Native compilation for testing:
The cmake build system allows us to easily swap C compilers and we can use that
to our advantage. Compiling for the host's system architecture allows to run
unit testing locally and check the code for logic correctness. We can therefore
disable `CROSS_COMPILE` and run tests as shown below:
```sh
mkdir buildt && cd buildt
cmake -DCROSS_COMPILE=0 ..
make && make test
```

cmake_minimum_required (VERSION 3.5)
enable_testing()

project(Pandos+)

find_program(MIPSEL_CC mipsel-linux-gnu-gcc)

# Cross-compile for the target environment by default
# Can be disabled to compile with the system's `cc` and run unit tests locally
if(NOT DEFINED CROSS_COMPILE)
    set(CROSS_COMPILE TRUE)
endif()

# Locate umps3 header/misc files
if(NOT DEFINED UMPS3_DIR_PREFIX)
    set(UMPS3_DIR_PREFIX /usr)
    if(EXISTS /usr/local/bin/umps3)
        set(UMPS3_DIR_PREFIX /usr/local)
    endif()
endif()

if(CROSS_COMPILE)
# flags needed to compile to an umps3 binary flags
    set(CFLAGS_LANG -ffreestanding -ansi)
    set(CFLAGS_MIPS -mips1 -mabi=32 -mno-gpopt -G 0 -mno-abicalls -fno-pic -mfp32)
    set(CFLAGS_ALL ${CFLAGS_LANG} ${CFLAGS_MIPS} -pedantic -Wall -O0)
    set(LDFLAGS_ALL -G 0 -nostdlib -T ${UMPS3_DIR_PREFIX}/share/umps3/umpscore.ldscript)
else()
    set(CFLAGS_ALL -pedantic -Wall)
endif()

# The library code for all project phases, as a subproject
add_subdirectory(os)
# Unit testing for the shared `os` subproject
if(NOT CROSS_COMPILE)
    add_subdirectory(test)
endif()

if(CROSS_COMPILE)
    add_subdirectory(phase1)
    # add_subdirectory(phase2)
    # add_subdirectory(phase3)
endif()

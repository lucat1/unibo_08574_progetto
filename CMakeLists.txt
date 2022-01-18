cmake_minimum_required (VERSION 3.5)
project(Pandos+ VERSION 0.1 LANGUAGES C ASM)
enable_testing()

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
    set(CFLAGS_LANG -Werror -Wall -ffreestanding -O0 -std=gnu99)
    set(CFLAGS_MIPS -mips1 -mabi=32 -mno-gpopt -G0 -fpic -mfp32) # Changed flags: -mno-abicalls -fno-pic
    set(CFLAGS_ALL ${CFLAGS_LANG} ${CFLAGS_MIPS})
    set(LDFLAGS_ALL -G0 -nostdlib -T ${UMPS3_DIR_PREFIX}/share/umps3/umpscore.ldscript)
else()
    set(CFLAGS_ALL ${CFLAGS_LANG})
endif()

# PLEASE NOTE: subprojects are ignored as needed, that is, based on the target
# architecture. That said, we want our development tools (namely the clangd LSP)
# to know about all files contained in all subprojects. So when cmake is asked
# to export metadata needed for them we add all subprojects eitherway. Clearly
# this configuration won't be suitable for production usage, so please don't use
# any Makefile generated via -DCMAKE_EXPORT_COMPILE_COMMANDS=1 to compile
# kernels or test libraries.

# The library code for all project phases, as a subproject
add_subdirectory(os)
# Unit testing for the shared `os` subproject
if(NOT CROSS_COMPILE OR CMAKE_EXPORT_COMPILE_COMMANDS)
    add_subdirectory(test)
endif()

if(CROSS_COMPILE OR CMAKE_EXPORT_COMPILE_COMMANDS)
    add_subdirectory(phase1)
    # add_subdirectory(phase2)
    # add_subdirectory(phase3)
endif()

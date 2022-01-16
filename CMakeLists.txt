cmake_minimum_required (VERSION 3.5)

project(Pandos+)

find_program(MIPSEL_GCC mipsel-linux-gnu-gcc)

# Locate umps3 header/misc files
if(NOT UMPS3_DIR_PREFIX)
    set(UMPS3_DIR_PREFIX /usr)
    if(EXISTS /usr/local/bin/umps3)
        set(UMPS3_DIR_PREFIX /usr/local)
    endif()
endif()

# flags needed to compile to an umps3 binary flags
set(CFLAGS_LANG -ffreestanding -ansi)
set(CFLAGS_MIPS -mips1 -mabi=32 -mno-gpopt -G 0 -mno-abicalls -fno-pic -mfp32)
set(CFLAGS_ALL ${CFLAGS_LANG} ${CFLAGS_MIPS} -Wall -O0)
set(LDFLAGS_ALL -G 0 -nostdlib -T ${UMPS3_DIR_PREFIX}/share/umps3/umpscore.ldscript)

# The library code for all project phases, as a subproject
add_subdirectory(os)

add_subdirectory(phase1)

cmake_minimum_required (VERSION 3.5)
project(Pandos+ VERSION 0.1 LANGUAGES C ASM)
enable_testing()


# Cross-compile for the target environment by default
# Can be disabled to compile with the system's `cc` and run unit tests locally
option(CROSS_COMPILE "Cross compile for the MIPS architecture" ON)
option(DEBUG "Enable GDB debug flags" OFF)
set(XT_PRG_PREFIX "mipsel-linux-gnu-" CACHE STRING "GCC cross compiler prefix")

find_program(MIPSEL_CC ${XT_PRG_PREFIX}gcc REQUIRED)
find_program(MIPSEL_LD ${XT_PRG_PREFIX}ld REQUIRED)

# Locate the UMPS3 header/obj files
if(NOT DEFINED UMPS3_DIR_PREFIX)
    set(UMPS3_DIR_PREFIX /usr)
    if(EXISTS /usr/local/bin/umps3)
        set(UMPS3_DIR_PREFIX /usr/local)
    endif()
endif()

set(CFLAGS_LANG -pedantic -ffreestanding -Werror -Wall -ansi -std=gnu99)
if(CROSS_COMPILE)
    # Compile the kernel binary to a bare metal executable with these flags
    set(CFLAGS_MIPS -mips1 -mabi=32 -mno-gpopt -EL -G0 -O0 -mno-abicalls -fno-pic -mfp32)
    set(CFLAGS_ALL ${CMAKE_C_FLAGS} ${CFLAGS_LANG} ${CFLAGS_MIPS})
    set(LDFLAGS_ALL ${LDFLAGS_ALL} -G0 -nostdlib -T ${UMPS3_DIR_PREFIX}/share/umps3/umpscore.ldscript -melf32ltsmip)
else()
    set(CFLAGS_ALL ${CMAKE_C_FLAGS} ${CFLAGS_LANG})
endif()

if(DEBUG)
    set(CFLAGS_ALL ${CFLAGS_ALL} -ggdb)
endif()

# PLEASE NOTE: subprojects are ignored as needed, that is, based on the target
# architecture. That said, we want our development tools (namely the clangd LSP)
# to know about all files contained in all subprojects. So when cmake is asked
# to export metadata needed for them we add all subprojects eitherway. Clearly
# this configuration won't be suitable for production usage, so please don't use
# any Makefile generated via -DCMAKE_EXPORT_COMPILE_COMMANDS=1 to compile
# kernels or test libraries.

# Unit testing for the shared `os` subproject
if(NOT CROSS_COMPILE)
    message(STATUS "Cross compilation disabled: testing enabled")
    add_compile_definitions(PANDOS_TESTING=1)
    add_subdirectory(test)
endif()

# The library code for all project phases, as a subproject
add_subdirectory(os)

if(CROSS_COMPILE)
    add_subdirectory(phase1)
    add_subdirectory(phase2)
    add_subdirectory(phase3)
endif()

option(BUILD_DOC "Documentation" ON)
find_package(Doxygen)
if(DOXYGEN_FOUND)
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating documentation"
        VERBATIM)
else()
    message(STATUS "Doxygen missing: documentation will not be generated")
endif()

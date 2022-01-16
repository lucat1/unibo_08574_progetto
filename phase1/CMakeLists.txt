project(phase1)
add_executable(${PROJECT_NAME} p1test.c)

set(CMAKE_C_COMPILER mipsel-linux-gnu-gcc)

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

target_compile_options(${PROJECT_NAME} PRIVATE ${CFLAGS_ALL})
target_link_options(${PROJECT_NAME} PRIVATE ${LDFLAGS_ALL})
target_include_directories(${PROJECT_NAME} PUBLIC ${UMPS3_DIR_PREFIX}/include/umps3)
target_link_libraries(${PROJECT_NAME}
    sub::os
    # Object files with utility functions from umps3
    ${UMPS3_DIR_PREFIX}/lib/umps3/crtso.o
    ${UMPS3_DIR_PREFIX}/lib/umps3/libumps.o
)

# convert the elf output binary into an executable umps3 kernel
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND umps3-elf2umps -k ${PROJECT_NAME}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Converting ELF ${PROJECT_NAME} into UMPS3 Kernel..."
)

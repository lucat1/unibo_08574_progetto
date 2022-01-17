project(phase1)
add_executable(${PROJECT_NAME}
    src/p1test.c
    /usr/share/umps3/libumps.S
    /usr/share/umps3/crtso.S
)

set(CMAKE_C_COMPILER ${MIPSEL_CC})
set(CMAKE_ASM_COMPILER ${MIPSEL_CC})
target_compile_options(${PROJECT_NAME} PRIVATE ${CFLAGS_ALL})
target_link_options(${PROJECT_NAME} PRIVATE ${LDFLAGS_ALL})
target_include_directories(${PROJECT_NAME} PUBLIC ${UMPS3_DIR_PREFIX}/include/umps3)
target_link_libraries(${PROJECT_NAME}
    sub::os
)

# convert the elf output binary into an executable umps3 kernel
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND umps3-elf2umps -k ${PROJECT_NAME}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Converting ELF ${PROJECT_NAME} into UMPS3 Kernel..."
)

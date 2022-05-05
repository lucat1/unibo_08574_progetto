project(phase2)
add_executable(${PROJECT_NAME}
    src/processor.c
    src/devices.c
    test/p2test.c
    src/exception.c
    src/semaphores.c
    src/main.c
)

set(CMAKE_C_COMPILER ${MIPSEL_CC})
# Force the MIPS linker to avoid the GCC fronted that screws things up
set(CMAKE_C_LINK_EXECUTABLE "${MIPSEL_LD} <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
target_compile_options(${PROJECT_NAME} PRIVATE ${CFLAGS_ALL})
target_link_options(${PROJECT_NAME} PRIVATE ${LDFLAGS_ALL})
target_include_directories(${PROJECT_NAME}
  PUBLIC ${PROJECT_SOURCE_DIR}/include
  PUBLIC ${PROJECT_SOURCE_DIR}/test
  PUBLIC ${UMPS3_DIR_PREFIX}/include/umps3)
target_link_libraries(${PROJECT_NAME}
    sub::os
    sub::phase3
    sub::libumps
    sub::crtso
)

# convert the elf output binary into an executable umps3 kernel
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND umps3-elf2umps -k ${PROJECT_NAME}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Converting ELF ${PROJECT_NAME} into uMPS3 Kernel..."
)

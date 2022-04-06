project (os)
list(APPEND ASMS crtso libumps)
list(APPEND TRGTS ${PROJECT_NAME})

add_library(${PROJECT_NAME}
    src/asl.c
    src/pcb.c
    src/scheduler.c
    src/semaphores.c
    src/util.c
)
add_library(sub::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

if(CROSS_COMPILE)
    set(CMAKE_C_COMPILER ${MIPSEL_CC})
    # Force the MIPS linker to avoid the GCC fronted that screws things up
    set(CMAKE_C_LINK_EXECUTABLE "${MIPSEL_LD} <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
    set(CMAKE_ASM_COMPILER ${MIPSEL_CC})

    foreach(ASM ${ASMS})
        add_library(${ASM} /usr/share/umps3/${ASM}.S)
        add_library(sub::${ASM} ALIAS ${ASM})
        list(APPEND TRGTS ${ASM})
    endforeach()
endif()

foreach(TRGT ${TRGTS})
    target_compile_options(${TRGT} PRIVATE ${CFLAGS_ALL})
    target_include_directories(${TRGT}
        PUBLIC ${PROJECT_SOURCE_DIR}/include
        PUBLIC ${UMPS3_DIR_PREFIX}/include/umps3
    )
endforeach()

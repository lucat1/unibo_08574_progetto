project(phase3)
add_library(${PROJECT_NAME}
    src/pager.c
    src/storage.c
    src/support.c
    src/test.c
)

add_library(sub::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
set(CMAKE_C_COMPILER ${MIPSEL_CC})
# Force the MIPS linker to avoid the GCC fronted that screws things up
set(CMAKE_C_LINK_EXECUTABLE "${MIPSEL_LD} <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
set(CMAKE_ASM_COMPILER ${MIPSEL_CC})
target_compile_options(${PROJECT_NAME} PRIVATE ${CFLAGS_ALL})
target_include_directories(${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}/include
    PUBLIC ${UMPS3_DIR_PREFIX}/include/umps3
)
target_link_libraries(${PROJECT_NAME}
    sub::os
)

set(TEST_FILES
  fibEight
  fibEleven
  printerTest
  strConcat
  terminalReader
  terminalTest2
  terminalTest3
  terminalTest4
  terminalTest5
)

foreach(TST ${TEST_FILES})
    add_executable(${TST} test/${TST}.c test/print.c)
    add_test(${TST} ${TST})

    target_compile_options(${TST} PRIVATE ${CFLAGS_ALL})
    target_link_options(${TST} PRIVATE -G0 -nostdlib -T ${UMPS3_DIR_PREFIX}/share/umps3/umpsaout.ldscript)
    target_include_directories(${TST}
      PUBLIC ${PROJECT_SOURCE_DIR}/include
      PUBLIC ${UMPS3_DIR_PREFIX}/include/umps3)
    target_link_libraries(${TST}
        sub::libumps
        sub::crti
    )
    add_custom_command(TARGET ${TST} POST_BUILD
        COMMAND umps3-elf2umps -a ${TST}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Converting ELF ${TST} into uMPS3 Executable..."
    )
    add_custom_command(TARGET ${TST} POST_BUILD
        COMMAND umps3-mkdev -f ${TST}.umps ${TST}.aout.umps
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Converting uMPS3 binary ${TST} into uMPS3 flash device..."
    )
endforeach()

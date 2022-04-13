project (test)

set(TEST_FILES
    pcb
    asl
    util
    init
    scheduler
    syscall_process_lifecycle
    syscall_P_and_V
    syscall_get_cpu_time
    syscall_clock_wait
    syscall_get_process_id
    syscall_yield
)

foreach(TST ${TEST_FILES})
    set(TEST_NAME test_${TST})
    add_executable(${TEST_NAME} src/${TST}.c)
    add_test(${TST} ${TEST_NAME})

    target_compile_options(${TEST_NAME} PRIVATE ${CFLAGS_ALL})
    target_include_directories(${TEST_NAME}
        PUBLIC ${PROJECT_SOURCE_DIR}/include
        PUBLIC ${UMPS3_DIR_PREFIX}/include/umps3)
    target_link_libraries(${TEST_NAME}
        sub::os
        m
    )
endforeach()

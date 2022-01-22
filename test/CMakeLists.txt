project (test)

set(TEST_FILES pcb asl util)

foreach(TST ${TEST_FILES})
    set(TEST_NAME test_${TST})
    add_executable(${TEST_NAME} src/${TST}.c)
    add_test(${TST} ${TEST_NAME})

    target_include_directories(${TEST_NAME}
        PUBLIC ${PROJECT_SOURCE_DIR}/include
        PUBLIC ${UMPS3_DIR_PREFIX}/include/umps3)
    target_link_libraries(${TEST_NAME}
        sub::os
    )
endforeach()

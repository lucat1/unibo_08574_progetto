project (os)

add_library(${PROJECT_NAME}
    src/asl.c
    src/pcb.c
)
add_library(sub::os ALIAS ${PROJECT_NAME})

set(CMAKE_C_COMPILER ${MIPSEL_GCC})
target_compile_options(${PROJECT_NAME} PRIVATE ${CFLAGS_ALL})
target_include_directories(${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}/include
    PUBLIC ${UMPS3_DIR_PREFIX}/include/umps3
)

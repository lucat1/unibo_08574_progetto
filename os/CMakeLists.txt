project (os)

add_library(${PROJECT_NAME}
    src/asl.c
    src/pcb.c
)
add_library(sub::os ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}/include
)

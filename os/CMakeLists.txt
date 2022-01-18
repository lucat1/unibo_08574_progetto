project (os)

add_library(${PROJECT_NAME}
    src/asl.c
    src/pcb.c
)
add_library(sub::os ALIAS ${PROJECT_NAME})

if(CROSS_COMPILE)
    set(CMAKE_C_COMPILER ${MIPSEL_CC})
endif()
target_compile_options(${PROJECT_NAME} PRIVATE ${CFLAGS_ALL})
target_include_directories(${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}/include
    PUBLIC ${UMPS3_DIR_PREFIX}/include/umps3
)

option(BUILD_DOC "Documentation" ON)
find_package(Doxygen)
if (DOXYGEN_FOUND)
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/../docs/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    add_custom_target( docs ALL
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating documentation"
        VERBATIM )
else (DOXYGEN_FOUND)
  message(WARNING "Doxygen missing: documentation will not be generated")
endif (DOXYGEN_FOUND)

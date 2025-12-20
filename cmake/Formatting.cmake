find_program(CLANG_FORMAT_EXE NAMES clang-format)

if(CLANG_FORMAT_EXE)
    file(GLOB_RECURSE FORMAT_SOURCES CONFIGURE_DEPENDS
        "${PROJECT_SOURCE_DIR}/src/*.cpp"
        "${PROJECT_SOURCE_DIR}/include/*.hpp"
        "${PROJECT_SOURCE_DIR}/tests/*.cpp"
    )

    add_custom_target(format
        COMMAND ${CLANG_FORMAT_EXE} -i ${FORMAT_SOURCES}
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        COMMENT "Warden: Running clang-format on all sources..."
        COMMAND_EXPAND_LISTS
        VERBATIM
    )
    
    message(STATUS "Clang-format found: ${CLANG_FORMAT_EXE}")
else()
    message(WARNING "Clang-format not found! Code formatting target will not be available.")
endif()
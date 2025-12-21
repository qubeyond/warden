include(GoogleTest)

function(create_test NAME SOURCES)
    add_executable(${NAME} ${SOURCES})
    
    target_link_libraries(${NAME} PRIVATE 
        warden_core 
        warden_system
        GTest::gtest_main
    )
    
    target_include_directories(${NAME} PRIVATE "${PROJECT_SOURCE_DIR}/include")

    gtest_discover_tests(${NAME})
endfunction()
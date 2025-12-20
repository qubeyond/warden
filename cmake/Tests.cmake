include(GoogleTest)

function(add_warden_test NAME SOURCES)
    add_executable(${NAME} ${SOURCES})
    
    target_link_libraries(${NAME} PRIVATE 
        warden_core 
        GTest::gtest_main
    )
    
    target_include_directories(${NAME} PRIVATE "${PROJECT_SOURCE_DIR}/include")
    gtest_discover_tests(${NAME})
endfunction()
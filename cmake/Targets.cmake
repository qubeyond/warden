file(GLOB_RECURSE WARDEN_CORE_SOURCES "${PROJECT_SOURCE_DIR}/src/services/*.cpp")
list(REMOVE_ITEM WARDEN_CORE_SOURCES "${PROJECT_SOURCE_DIR}/src/services/cli_service.cpp")

add_library(warden_core SHARED ${WARDEN_CORE_SOURCES})
set_target_properties(warden_core PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_include_directories(warden_core PUBLIC "${PROJECT_SOURCE_DIR}/include")
target_link_libraries(warden_core PUBLIC  
    OpenSSL::Crypto 
    nlohmann_json::nlohmann_json
    XGBoost::xgboost
    Boost::boost
)

add_executable(warden_cli 
    src/cli/main.cpp 
    src/services/cli_service.cpp
)
target_link_libraries(warden_cli PRIVATE 
    warden_core 
    CLI11::CLI11
)

function(copy_project_resources TARGET_NAME)
    set(RESOURCES "configs" "models")
    foreach(RES ${RESOURCES})
        if(EXISTS "${PROJECT_SOURCE_DIR}/${RES}/")
            add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${PROJECT_SOURCE_DIR}/${RES}"
                "$<TARGET_FILE_DIR:${TARGET_NAME}>/${RES}"
                COMMENT "Copying ${RES} for ${TARGET_NAME}"
            )
        endif()
    endforeach()
endfunction()

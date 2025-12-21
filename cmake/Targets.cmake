file(GLOB_RECURSE WARDEN_CORE_SOURCES CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/src/services/*.cpp")
file(GLOB_RECURSE CLI_SERVICE_PATH CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/src/services/*/cli_service.cpp")

if(CLI_SERVICE_PATH)
    list(REMOVE_ITEM WARDEN_CORE_SOURCES ${CLI_SERVICE_PATH})
endif()

add_library(warden_core SHARED ${WARDEN_CORE_SOURCES})
set_target_properties(warden_core PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_include_directories(warden_core PUBLIC "${PROJECT_SOURCE_DIR}/include")
target_link_libraries(warden_core PUBLIC  
    OpenSSL::Crypto 
    nlohmann_json::nlohmann_json
    XGBoost::xgboost
    Boost::boost
    magic
)

add_executable(warden_cli 
    src/cli/main.cpp 
    ${CLI_SERVICE_PATH}
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

copy_project_resources(warden_cli)
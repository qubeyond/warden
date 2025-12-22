file(GLOB_RECURSE CORE_SOURCES CONFIGURE_DEPENDS 
    "${PROJECT_SOURCE_DIR}/src/services/core/*.cpp"
    "${PROJECT_SOURCE_DIR}/src/services/detector_service.cpp"
    "${PROJECT_SOURCE_DIR}/src/services/system/config.cpp"
)

add_library(warden_core SHARED ${CORE_SOURCES})

target_include_directories(warden_core PUBLIC 
    "${PROJECT_SOURCE_DIR}/include"
)

target_link_libraries(warden_core PUBLIC 
    OpenSSL::Crypto 
    nlohmann_json::nlohmann_json 
    XGBoost::xgboost 
    Boost::boost 
    magic
)

file(GLOB_RECURSE SYSTEM_SOURCES CONFIGURE_DEPENDS 
    "${PROJECT_SOURCE_DIR}/src/services/system/*.cpp"
)

list(REMOVE_ITEM SYSTEM_SOURCES "${PROJECT_SOURCE_DIR}/src/services/system/config.cpp")

add_library(warden_system SHARED ${SYSTEM_SOURCES})

target_include_directories(warden_system PUBLIC 
    "${PROJECT_SOURCE_DIR}/include"
)

target_link_libraries(warden_system PUBLIC 
    warden_core
)

file(GLOB_RECURSE CLI_SOURCES CONFIGURE_DEPENDS 
    "${PROJECT_SOURCE_DIR}/src/services/cli/*.cpp"
)

add_executable(warden_cli 
    src/cli/main.cpp 
    ${CLI_SOURCES}
)

target_link_libraries(warden_cli PRIVATE 
    warden_core 
    warden_system 
    CLI11::CLI11
)

function(copy_project_resources)
    set(RESOURCES "configs" "models")
    foreach(RES ${RESOURCES})
        if(EXISTS "${PROJECT_SOURCE_DIR}/${RES}/")
            file(COPY "${PROJECT_SOURCE_DIR}/${RES}" DESTINATION "${CMAKE_BINARY_DIR}")

            file(COPY "${PROJECT_SOURCE_DIR}/${RES}" DESTINATION "${CMAKE_BINARY_DIR}/tests")
        endif()
    endforeach()
endfunction()

copy_project_resources()
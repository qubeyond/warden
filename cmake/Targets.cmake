file(GLOB_RECURSE WARDEN_CORE_SOURCES "${PROJECT_SOURCE_DIR}/src/services/*.cpp")

add_library(warden_core SHARED ${WARDEN_CORE_SOURCES})
set_target_properties(warden_core PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_include_directories(warden_core PUBLIC "${PROJECT_SOURCE_DIR}/include")
target_link_libraries(warden_core PUBLIC  
    OpenSSL::Crypto 
    nlohmann_json::nlohmann_json
    XGBoost::xgboost
)

add_executable(warden_cli src/cli/main.cpp)
target_link_libraries(warden_cli PRIVATE warden_core)

function(copy_project_resources)
    set(RESOURCES "configs" "models")
    foreach(RES ${RESOURCES})
        if(EXISTS "${PROJECT_SOURCE_DIR}/${RES}/")
            file(COPY "${PROJECT_SOURCE_DIR}/${RES}/" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/${RES}")
            message(STATUS "Resource copied: ${RES}")
        endif()
    endforeach()
endfunction()

copy_project_resources()

if(TARGET format)
    add_dependencies(warden_core format)
endif()
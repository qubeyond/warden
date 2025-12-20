find_path(XGBOOST_INCLUDE_DIR NAMES xgboost/c_api.h)
find_library(XGBOOST_LIBRARY NAMES xgboost)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(XGBoost DEFAULT_MSG XGBOOST_LIBRARY XGBOOST_INCLUDE_DIR)

if(XGBOOST_FOUND)
    set(XGBOOST_LIBRARIES ${XGBOOST_LIBRARY})
    set(XGBOOST_INCLUDE_DIRS ${XGBOOST_INCLUDE_DIR})
    
    if(NOT TARGET XGBoost::xgboost)
        add_library(XGBoost::xgboost UNKNOWN IMPORTED)
        set_target_properties(XGBoost::xgboost PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${XGBOOST_INCLUDE_DIRS}"
            IMPORTED_LOCATION "${XGBOOST_LIBRARIES}"
        )
    endif()
endif()
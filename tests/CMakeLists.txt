include(Tests)

create_test(config_unit_test
    "unit/config_service_test.cpp"
)

create_test(feature_unit_test
    "unit/feature_service_test.cpp"
)

create_test(model_unit_test
    "unit/model_service_test.cpp"
)

create_test(scan_unit_test 
    "unit/scan_service_test.cpp"
)

create_test(detector_unit_test
    "unit/detector_service_test.cpp"
)

create_test(detector_intg_test
    "integration/detector_service_test.cpp"
)
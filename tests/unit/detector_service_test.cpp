#include "services/core/detector_service.hpp"

#include <gtest/gtest.h>

#include "services/system/config_service.hpp"

using namespace warden::services;

TEST(DetectorServiceUnitTest, LogicVerdictCalculation) {
    std::string a = "configs/app_config.json";
    std::string m = "configs/model_config_v2.json";
    std::string p = "configs/properties.json";

    ConfigService cs(a, m, p);
    ScanService ss(cs);
    FeatureService fs(cs);
    ModelService ms(cs);
    DetectorService ds(ss, fs, ms);

    EXPECT_NO_THROW(ds.process_file("detector_unit_test", 0.5f));
}
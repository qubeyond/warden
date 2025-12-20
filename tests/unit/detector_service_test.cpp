#include "services/detector_service.hpp"

#include <gtest/gtest.h>

using namespace warden::services;
using namespace warden::common;

TEST(DetectorServiceUnitTest, LogicVerdictCalculation) {
    ScanService ss;
    FeatureService fs;
    ModelService ms("../models/ransomware_model_v1.json");
    DetectorService ds(ss, fs, ms);
}
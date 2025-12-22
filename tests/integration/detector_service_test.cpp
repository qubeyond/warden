#include "services/core/detector_service.hpp"

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <memory>
#include <vector>

#include "services/core/feature_service.hpp"
#include "services/core/model_service.hpp"
#include "services/core/scan_service.hpp"
#include "services/system/config_service.hpp"

using namespace warden::services;
using namespace warden::common;

class DetectorIntegrationTest : public ::testing::Test {
   protected:
    std::shared_ptr<ConfigService> cs;
    std::unique_ptr<ScanService> ss;
    std::unique_ptr<FeatureService> fs;
    std::unique_ptr<ModelService> ms;

    void SetUp() override {
        cs = ConfigService::load("configs/app_config.json", "configs/model_config_v2.json",
                                 "configs/properties.json");
        ss = std::make_unique<ScanService>(*cs);
        fs = std::make_unique<FeatureService>(*cs);
        ms = std::make_unique<ModelService>(*cs);
    }

    void create_file(const std::string& p, size_t s, uint8_t b) {
        std::ofstream f(p, std::ios::binary);
        std::vector<uint8_t> d(s, b);
        f.write(reinterpret_cast<const char*>(d.data()), s);
    }
};

TEST_F(DetectorIntegrationTest, BoundarySmallFile) {
    std::string p = "test_1b.bin";
    create_file(p, 1, 0x00);
    DetectorService ds(*ss, *fs, *ms);
    auto res = ds.process_file(p, 0.5f);
    EXPECT_EQ(res.total_chunks, 1);
    std::remove(p.c_str());
}

TEST_F(DetectorIntegrationTest, NonExistentFile) {
    DetectorService ds(*ss, *fs, *ms);
    auto res = ds.process_file("not_found.exe", 0.5f);
    EXPECT_EQ(res.verdict, Verdict::UNKNOWN);
}
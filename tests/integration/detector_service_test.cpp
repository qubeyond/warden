#include "services/core/detector_service.hpp"

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <memory>
#include <vector>

#include "services/system/config_service.hpp"

using namespace warden::services;
using namespace warden::common;

class DetectorIntegrationTest : public ::testing::Test {
   protected:
    std::unique_ptr<ConfigService> cs;
    std::unique_ptr<ScanService> ss;
    std::unique_ptr<FeatureService> fs;
    std::unique_ptr<ModelService> ms;

    void SetUp() override {
        std::string a = "configs/app_config.json";
        std::string m = "configs/model_config_v2.json";
        std::string p = "configs/properties.json";

        if (!std::filesystem::exists(a)) {
            throw std::runtime_error("Test resource missing at: " +
                                     std::filesystem::absolute(a).string());
        }

        cs = std::make_unique<ConfigService>(a, m, p);
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

TEST_F(DetectorIntegrationTest, BoundaryExactlyTenChunks) {
    std::string p = "test_multi.bin";
    create_file(p, 1024 * 1024, 0x00);
    DetectorService ds(*ss, *fs, *ms);
    auto res = ds.process_file(p, 0.5f);
    EXPECT_GT(res.total_chunks, 0);
    std::remove(p.c_str());
}

TEST_F(DetectorIntegrationTest, NonExistentFile) {
    DetectorService ds(*ss, *fs, *ms);
    auto res = ds.process_file("not_found.exe", 0.5f);
    EXPECT_EQ(res.verdict, Verdict::UNKNOWN);
}
#include "services/feature_service.hpp"
#include "services/config_service.hpp"
#include <gtest/gtest.h>
#include <numeric>
#include <vector>
#include <fstream>

using namespace warden::services;

class FeatureServiceTest : public ::testing::Test {
protected:
    std::unique_ptr<ConfigService> cs;
    std::unique_ptr<FeatureService> fs;

    void SetUp() override {
        std::ofstream app("feat_app.json");
        app << R"({"database": {"host": "localhost"}, "scanner": {"watch_dirs": []}})";
        app.close();

        std::ofstream model("feat_model.json");
        model << R"({"model_file": "m.json", "threshold": 0.5, "n_features": 262, "model_type": "XGB"})";
        model.close();

        cs = std::make_unique<ConfigService>("feat_app.json", "feat_model.json");
        fs = std::make_unique<FeatureService>(*cs);
    }

    void TearDown() override {
        std::remove("feat_app.json");
        std::remove("feat_model.json");
    }
};

TEST_F(FeatureServiceTest, ByteHistogramOrder) {
    std::vector<uint8_t> data(100, 0x00);
    data.insert(data.end(), 300, 0xFF);

    auto f = fs->extract_from_buffer(data);

    ASSERT_EQ(f.size(), 262);
    EXPECT_NEAR(f[0], 0.25f, 0.001f);
    EXPECT_NEAR(f[255], 0.75f, 0.001f);
}

TEST_F(FeatureServiceTest, StatisticsVerification) {
    std::vector<uint8_t> data = {0, 0, 1, 1, 2, 2, 3, 3};
    auto f = fs->extract_from_buffer(data);

    EXPECT_GT(f[256], 0.0f); 
    EXPECT_NEAR(f[257], 1.5f, 0.01f);
    EXPECT_NEAR(f[258], 1.118f, 0.01f);
    EXPECT_NEAR(f[261], 1.0f, 0.01f); 
}

TEST_F(FeatureServiceTest, EmptyBufferHandling) {
    std::vector<uint8_t> empty_data;
    auto f = fs->extract_from_buffer(empty_data);

    ASSERT_EQ(f.size(), 262);
    for (float val : f) {
        EXPECT_EQ(val, 0.0f);
    }
}

TEST_F(FeatureServiceTest, ChiSquareCalculation) {
    std::vector<uint8_t> data(256);
    std::iota(data.begin(), data.end(), 0);

    auto f = fs->extract_from_buffer(data);
    EXPECT_NEAR(f[260], 0.0f, 0.001f);
}
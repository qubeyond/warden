#include "services/model_service.hpp"
#include "services/config_service.hpp"
#include <gtest/gtest.h>
#include <vector>
#include <fstream>

using namespace warden::services;

class ModelServiceTest : public ::testing::Test {
protected:
    std::unique_ptr<ConfigService> cs;

    void SetUp() override {
        std::ofstream app("mod_app.json");
        app << R"({"database": {"host": "localhost"}, "scanner": {"watch_dirs": []}})";
        app.close();

        std::ofstream model("mod_model.json");
        model << R"({
            "model_file": "../models/ransomware_model_v2.json", 
            "threshold": 0.5, 
            "n_features": 262, 
            "model_type": "XGB"
        })";
        model.close();

        cs = std::make_unique<ConfigService>("mod_app.json", "mod_model.json");
    }

    void TearDown() override {
        std::remove("mod_app.json");
        std::remove("mod_model.json");
    }
};

TEST_F(ModelServiceTest, InferenceLogic) {
    ModelService ms(*cs);

    std::vector<float> zero_features(262, 0.0f);
    float score_low = ms.predict(zero_features);
    EXPECT_GE(score_low, 0.0f);
    EXPECT_LE(score_low, 1.0f);

    std::vector<float> high_entropy_features(262, 0.0f);
    for (int i = 0; i < 256; ++i) high_entropy_features[i] = 1.0f / 256.0f;
    high_entropy_features[256] = 8.0f;

    float score_high = ms.predict(high_entropy_features);
    EXPECT_GT(score_high, score_low);
}
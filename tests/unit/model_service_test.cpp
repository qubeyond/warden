#include "services/model_service.hpp"
#include "services/config_service.hpp"
#include <gtest/gtest.h>
#include <vector>
#include <memory>

using namespace warden::services;

class ModelServiceTest : public ::testing::Test {
protected:
    std::unique_ptr<ConfigService> cs;

    void SetUp() override {
        std::string app_path = "configs/app_config.json";
        std::string mod_path = "configs/model_config_v2.json";
        std::string prp_path = "configs/properties.json";

        cs = std::make_unique<ConfigService>(app_path, mod_path, prp_path);
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

    EXPECT_GE(score_high, 0.0f);
    EXPECT_LE(score_high, 1.0f);
}
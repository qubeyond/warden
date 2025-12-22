#include "services/core/model_service.hpp"

#include <gtest/gtest.h>

#include <memory>
#include <vector>

#include "common/config.hpp"

using namespace warden::services;

class ModelServiceTest : public ::testing::Test {
   protected:
    std::shared_ptr<ConfigManager> cs;

    void SetUp() override {
        cs = ConfigManager::load("configs/app_config.json", "configs/model_config_v2.json",
                                 "configs/properties.json");
    }
};

TEST_F(ModelServiceTest, InferenceLogic) {
    ModelService ms(*cs);
    std::vector<float> zero_features(262, 0.0f);
    float score_low = ms.predict(zero_features);
    EXPECT_GE(score_low, 0.0f);
    EXPECT_LE(score_low, 1.0f);
}
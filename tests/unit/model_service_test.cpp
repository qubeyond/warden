#include "services/model_service.hpp"

#include <gtest/gtest.h>

#include <vector>

using namespace warden::services;

TEST(ModelServiceTest, InferenceLogic) {
    ModelService ms("../models/ransomware_model_v1.json");

    std::vector<float> zero_features(263, 0.0f);
    float score_low = ms.predict(zero_features);
    EXPECT_GE(score_low, 0.0f);
    EXPECT_LT(score_low, 0.5f);

    std::vector<float> high_entropy_features(263, 0.0f);
    for (int i = 0; i < 256; ++i) high_entropy_features[i] = 1.0f / 256.0f;
    high_entropy_features[256] = 8.0f;

    float score_high = ms.predict(high_entropy_features);
    EXPECT_GT(score_high, score_low);
}
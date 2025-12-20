#include <gtest/gtest.h>

#include <vector>

#include "services/model_service.hpp"

using namespace warden::services;

TEST(ModelServiceTest, InferenceLogic) {
    // ВАЖНО: убедись, что файл модели скопирован в build/models/
    ModelService ms("../models/ransomware_model_v1.json");

    // 1. Тест на пустом/нулевом векторе
    std::vector<float> zero_features(263, 0.0f);
    float score_low = ms.predict(zero_features);
    EXPECT_GE(score_low, 0.0f);
    EXPECT_LT(score_low, 0.5f);  // Ожидаем низкий шанс малвари

    // 2. Тест на "шумном" векторе (имитация зашифрованных данных/высокой энтропии)
    std::vector<float> high_entropy_features(263, 0.0f);
    for (int i = 0; i < 256; ++i) high_entropy_features[i] = 1.0f / 256.0f;  // Плоская гистограмма
    high_entropy_features[256] = 8.0f;  // Максимальная энтропия

    float score_high = ms.predict(high_entropy_features);
    // Обычно высокая энтропия + плоская гистограмма = признак шифровальщика
    EXPECT_GT(score_high, score_low);
}
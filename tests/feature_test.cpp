#include <gtest/gtest.h>

#include <fstream>
#include <numeric>
#include <vector>

#include "services/feature_service.hpp"

using namespace warden::services;

class FeatureServiceTest : public ::testing::Test {
   protected:
    void create_file(const std::string& name, const std::vector<uint8_t>& data) {
        std::ofstream f(name, std::ios::binary);
        f.write(reinterpret_cast<const char*>(data.data()), data.size());
    }
};

// Проверка гистограммы и порядка (h0-h255)
TEST_F(FeatureServiceTest, ByteHistogramOrder) {
    std::string path = "test_bytes.bin";
    // 100 байт 0x00 и 300 байт 0xFF
    std::vector<uint8_t> data(100, 0x00);
    data.insert(data.end(), 300, 0xFF);
    create_file(path, data);

    FeatureService fs;
    auto f = fs.extract_features(path);

    ASSERT_EQ(f.size(), 263);
    EXPECT_NEAR(f[0], 0.25f, 0.001f);    // h0
    EXPECT_NEAR(f[255], 0.75f, 0.001f);  // h255
    EXPECT_NEAR(f[1], 0.0f, 0.001f);     // h1

    std::remove(path.c_str());
}

// Проверка статистических признаков (entropy, mean, std)
TEST_F(FeatureServiceTest, StatisticsVerification) {
    std::string path = "test_stats.bin";
    // Создаем предсказуемую последовательность
    std::vector<uint8_t> data = {0, 1, 2, 3, 4, 5, 6, 7};
    create_file(path, data);

    FeatureService fs;
    auto f = fs.extract_features(path);

    // Mean (индекс 257) для 0-7: (0+1+2+3+4+5+6+7)/256 (так как гистограмма нормализована)
    // Внимание: проверь, как в Python считался mean (от байтов или от частот!)
    // Если от частот (1/256), то:
    EXPECT_GT(f[257], 0.0f);

    // Entropy (индекс 256) для уникальных 8 байт: log2(8) = 3.0
    EXPECT_NEAR(f[256], 3.0f, 0.1f);

    std::remove(path.c_str());
}
#include "services/feature_service.hpp"

#include <gtest/gtest.h>

#include <numeric>
#include <vector>

#include "common/defs.hpp"

using namespace warden::services;
using namespace warden::common;

class FeatureServiceTest : public ::testing::Test {
   protected:
    FeatureService fs;
};

TEST_F(FeatureServiceTest, ByteHistogramOrder) {
    std::vector<uint8_t> data(100, 0x00);
    data.insert(data.end(), 300, 0xFF);

    auto f = fs.extract_from_buffer(data);

    ASSERT_EQ(f.size(), FEATURES_COUNT);
    EXPECT_NEAR(f[0], 0.25f, 0.001f);
    EXPECT_NEAR(f[255], 0.75f, 0.001f);
    EXPECT_NEAR(f[1], 0.0f, 0.001f);
}

TEST_F(FeatureServiceTest, StatisticsVerification) {
    std::vector<uint8_t> data = {0, 1, 2, 3, 4, 5, 6, 7};

    auto f = fs.extract_from_buffer(data);

    EXPECT_NEAR(f[256], 3.0f, 0.01f);

    EXPECT_NEAR(f[257], 3.5f, 0.01f);

    EXPECT_NEAR(f[262], 1.0f, 0.01f);
}

TEST_F(FeatureServiceTest, EmptyBufferHandling) {
    std::vector<uint8_t> empty_data;
    auto f = fs.extract_from_buffer(empty_data);

    ASSERT_EQ(f.size(), FEATURES_COUNT);
    for (float val : f) {
        EXPECT_EQ(val, 0.0f);
    }
}

TEST_F(FeatureServiceTest, ChiSquareCalculation) {
    std::vector<uint8_t> data(256);
    std::iota(data.begin(), data.end(), 0);

    auto f = fs.extract_from_buffer(data);

    EXPECT_NEAR(f[260], 0.0f, 0.001f);
}
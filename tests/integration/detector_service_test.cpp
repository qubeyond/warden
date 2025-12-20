#include "services/detector_service.hpp"

#include <gtest/gtest.h>

#include <fstream>

using namespace warden::services;
using namespace warden::common;

class DetectorIntegrationTest : public ::testing::Test {
   protected:
    ScanService ss;
    FeatureService fs;
    ModelService ms{"../models/ransomware_model_v1.json"};

    void create_file(const std::string& p, size_t s, uint8_t b) {
        std::ofstream f(p, std::ios::binary);
        std::vector<uint8_t> d(s, b);
        f.write(reinterpret_cast<const char*>(d.data()), s);
    }
};

TEST_F(DetectorIntegrationTest, BoundarySmallFile) {
    std::string p = "1byte.bin";
    create_file(p, 1, 0x00);
    DetectorService ds(ss, fs, ms);
    auto res = ds.process_file(p, 0.5f);

    EXPECT_EQ(res.total_chunks, 1);
    EXPECT_LE(res.max_probability, 1.0f);

    std::remove(p.c_str());
}

TEST_F(DetectorIntegrationTest, BoundaryExactlyTenChunks) {
    std::string p = "multi_chunk.bin";
    create_file(p, CHUNK_SIZE * 20, 0x00);
    DetectorService ds(ss, fs, ms);
    auto res = ds.process_file(p, 0.5f);

    EXPECT_GE(res.total_chunks, 11);

    std::remove(p.c_str());
}

TEST_F(DetectorIntegrationTest, NonExistentFile) {
    DetectorService ds(ss, fs, ms);
    auto res = ds.process_file("missing_file.exe", 0.5f);
    EXPECT_EQ(res.verdict, Verdict::UNKNOWN);
}
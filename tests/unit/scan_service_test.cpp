#include "services/scan_service.hpp"

#include <gtest/gtest.h>

#include <fstream>
#include <vector>

#include "common/defs.hpp"

using namespace warden::services;
using namespace warden::common;

class ScanServiceTest : public ::testing::Test {
   protected:
    void create_dummy(const std::string& p, size_t s) {
        std::ofstream f(p, std::ios::binary);
        std::vector<uint8_t> d(s, 0x41);
        f.write(reinterpret_cast<const char*>(d.data()), s);
    }
};

TEST_F(ScanServiceTest, HandlesSmallFilePadding) {
    std::string p = "small.bin";
    create_dummy(p, 100);
    ScanService ss;
    auto c = ss.get_file_chunks(p);
    ASSERT_FALSE(c.empty());
    EXPECT_EQ(c[0].size(), CHUNK_SIZE);
    EXPECT_EQ(c[0][4095], 0x00);
    std::remove(p.c_str());
}

TEST_F(ScanServiceTest, CorrectChunkCount) {
    std::string p = "large.bin";
    create_dummy(p, CHUNK_SIZE * 20);
    ScanService ss;
    auto c = ss.get_file_chunks(p);
    EXPECT_GE(c.size(), 11);
    for (const auto& ch : c) {
        EXPECT_EQ(ch.size(), CHUNK_SIZE);
    }
    std::remove(p.c_str());
}
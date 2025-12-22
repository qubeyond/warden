#include "services/core/scan_service.hpp"

#include <gtest/gtest.h>

#include <fstream>
#include <vector>

#include "common/defs.hpp"
#include "services/system/config_service.hpp"

using namespace warden::services;
using namespace warden::common;
using namespace warden::common::constants;

class ScanServiceTest : public ::testing::Test {
   protected:
    std::shared_ptr<ConfigService> cs;

    void SetUp() override {
        std::ofstream app("s_app.json");
        app << "{}";
        app.close();
        std::ofstream mod("s_mod.json");
        mod << R"({"model_file":"m"})";
        mod.close();
        std::ofstream prp("s_prp.json");
        prp << R"({"scanner": {"min_chunks": 10, "max_chunks": 50}})";
        prp.close();
        cs = ConfigService::load("s_app.json", "s_mod.json", "s_prp.json");
    }

    void create_dummy(const std::string& p, size_t s) {
        std::ofstream f(p, std::ios::binary);
        std::vector<uint8_t> d(s, 0x41);
        f.write(reinterpret_cast<const char*>(d.data()), s);
    }

    void TearDown() override {
        std::remove("s_app.json");
        std::remove("s_mod.json");
        std::remove("s_prp.json");
    }
};

TEST_F(ScanServiceTest, HandlesSmallFilePadding) {
    std::string p = "small.bin";
    create_dummy(p, 100);
    ScanService ss(*cs);
    auto c = ss.get_file_chunks(p);
    ASSERT_FALSE(c.empty());
    EXPECT_EQ(c[0].size(), CHUNK_SIZE);
    std::remove(p.c_str());
}

TEST_F(ScanServiceTest, CorrectChunkCount) {
    std::string p = "large.bin";
    create_dummy(p, CHUNK_SIZE * 25);
    ScanService ss(*cs);
    auto c = ss.get_file_chunks(p);
    EXPECT_GE(c.size(), 10);
    std::remove(p.c_str());
}
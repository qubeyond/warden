#include "common/logger.hpp"

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <thread>

#include "common/config.hpp" 
#include "services/core/detector_service.hpp"

namespace fs = std::filesystem;

class LoggerTest : public ::testing::Test {
   protected:
    const std::string test_dir = "./test_logs";
    const std::string log_file = "./test_logs/warden.json";

    void SetUp() override {
        if (fs::exists(test_dir)) fs::remove_all(test_dir);
        fs::create_directory(test_dir);
    }
    void TearDown() override {
        if (fs::exists(test_dir)) fs::remove_all(test_dir);
    }
};

TEST_F(LoggerTest, LogsCorrectJsonFormat) {
    warden::services::LoggerConfig cfg;
    cfg.log_dir = test_dir;
    cfg.log_level = "all";
    cfg.max_file_size_mb = 1;
    cfg.max_files = 1;

    warden::common::Logger::init(cfg);

    warden::services::DetectionResult res;
    res.verdict = warden::common::Verdict::MALICIOUS;
    res.max_probability = 0.95f;
    res.suspicious_chunks = 10;
    res.total_chunks = 100;

    warden::common::Logger::log_detection("/tmp/test.exe", res);

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    ASSERT_TRUE(fs::exists(log_file));

    std::ifstream f(log_file);
    std::string line;
    std::getline(f, line);

    auto j = nlohmann::json::parse(line);
    EXPECT_EQ(j["path"], "/tmp/test.exe");
    EXPECT_EQ(j["verdict"], "MALICIOUS");
    EXPECT_NEAR(j["prob"].get<float>(), 0.95f, 0.001);
}
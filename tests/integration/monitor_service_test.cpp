#include "services/system/monitor_service.hpp"

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <thread>

#include "services/core/detector_service.hpp"
#include "services/core/feature_service.hpp"
#include "services/core/model_service.hpp"
#include "services/core/scan_service.hpp"
#include "services/system/config_service.hpp"

using namespace warden::services;
namespace fs = std::filesystem;

class TestObserver : public IReportObserver {
   public:
    std::atomic<bool> detected{false};
    std::string last_path;
    void notify_detection(const std::string& path, const DetectionResult& result) override {
        (void)result;
        last_path = path;
        detected = true;
    }
};

class MonitorIntegrationTest : public ::testing::Test {
   protected:
    const std::string test_dir = "test_watch_dir";
    void SetUp() override {
        if (fs::exists(test_dir)) fs::remove_all(test_dir);
        fs::create_directory(test_dir);
    }
    void TearDown() override {
        if (fs::exists(test_dir)) fs::remove_all(test_dir);
    }
};

TEST_F(MonitorIntegrationTest, DetectsNewFileWithFanotify) {
    auto cs = ConfigService::load("configs/app_config.json", "configs/model_config_v2.json",
                                  "configs/properties.json");
    ScanService ss(*cs);
    FeatureService fs_svc(*cs);
    ModelService ms(*cs);
    DetectorService ds(ss, fs_svc, ms);

    TestObserver observer;
    MonitorService monitor(ds, *cs, observer);

    monitor.start({fs::absolute(test_dir).string()});
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    std::string file_path = fs::absolute(test_dir + "/test_MALICIOUS.bin").string();
    std::string cmd = "dd if=/dev/urandom of=" + file_path + " bs=1024 count=1 2>/dev/null";

    int status = std::system(cmd.c_str());
    ASSERT_EQ(status, 0);

    bool success = false;
    for (int i = 0; i < 30; ++i) {
        if (observer.detected) {
            success = true;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    monitor.stop();
    EXPECT_TRUE(success);
}
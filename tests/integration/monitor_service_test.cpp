#include "services/system/monitor_service.hpp"

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <thread>

#include "services/core/detector_service.hpp"
#include "services/system/config_service.hpp"

using namespace warden::services;
namespace fs = std::filesystem;

class TestObserver : public IReportObserver {
   public:
    std::atomic<bool> detected{false};
    std::string last_path;

    void notify_detection(const std::string& path, const DetectionResult& result) override {
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
    ConfigService cs("configs/app_config.json", "configs/model_config_v2.json",
                     "configs/properties.json");

    ScanService ss(cs);
    FeatureService fs_svc(cs);
    ModelService ms(cs);
    DetectorService ds(ss, fs_svc, ms);

    TestObserver observer;
    MonitorService monitor(ds, cs, observer);

    try {
        monitor.start({fs::absolute(test_dir).string()});
    } catch (const std::runtime_error& e) {
        FAIL() << "Monitor failed: " << e.what();
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    std::string file_path = fs::absolute(test_dir + "/test_malware.bin").string();

    std::string cmd = "dd if=/dev/urandom of=" + file_path + " bs=1024 count=1 2>/dev/null";
    std::system(cmd.c_str());

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
    if (success) {
        EXPECT_TRUE(fs::equivalent(observer.last_path, file_path));
    }
}

TEST_F(MonitorIntegrationTest, HandlesRapidFileCreation) {
    ConfigService cs("configs/app_config.json", "configs/model_config_v2.json",
                     "configs/properties.json");
    ScanService ss(cs);
    FeatureService fs_svc(cs);
    ModelService ms(cs);
    DetectorService ds(ss, fs_svc, ms);

    TestObserver observer;
    MonitorService monitor(ds, cs, observer);

    try {
        monitor.start({fs::absolute(test_dir).string()});
    } catch (...) {
        FAIL() << "Monitor failed to start";
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    for (int i = 0; i < 5; ++i) {
        std::string path = fs::absolute(test_dir + "/file_" + std::to_string(i) + ".bin").string();
        std::string cmd = "echo 'data' > " + path;
        std::system(cmd.c_str());
    }

    bool success = false;
    for (int i = 0; i < 50; ++i) {
        if (observer.detected) {
            success = true;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    monitor.stop();
    EXPECT_TRUE(success);
}
#include "services/system/monitor_service.hpp"

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <thread>

#include "services/cli/cli_service.hpp"
#include "services/core/detector_service.hpp"
#include "services/system/config_service.hpp"

using namespace warden::services;

class TestObserver : public IReportObserver {
   public:
    bool detected = false;
    void notify_detection(const std::string&, const DetectionResult&) override {
        detected = true;
    }
};

TEST(MonitorIntegrationTest, DetectsNewFile) {
    std::string test_dir = "test_watch_dir";
    std::filesystem::create_directory(test_dir);

    ConfigService cs("configs/app_config.json", "configs/model_config_v2.json",
                     "configs/properties.json");
    ScanService ss(cs);
    FeatureService fs(cs);
    ModelService ms(cs);
    DetectorService ds(ss, fs, ms);
    TestObserver observer;
    MonitorService monitor(ds, cs, observer);

    std::thread t([&]() { monitor.start({test_dir}); });
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::ofstream f(test_dir + "/virus.bin");
    f << "X5O!P%@AP[4\\PZX54(P^)7CC)7}$EICAR-STANDARD-ANTIVIRUS-TEST-FILE!$H+H*";
    f.close();

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    monitor.stop();
    (void)std::system(("touch " + test_dir + "/stop").c_str());
    if (t.joinable()) t.join();

    EXPECT_TRUE(observer.detected);
    std::filesystem::remove_all(test_dir);
}
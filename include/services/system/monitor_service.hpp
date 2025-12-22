#pragma once

#include <atomic>
#include <cstdint>
#include <string>
#include <thread>
#include <vector>

#include "common/report_observer.hpp"

namespace warden::services {

class DetectorService;
class ConfigManager;

class MonitorService {
   public:
    MonitorService(DetectorService& ds, const ConfigManager& cs, IReportObserver& observer);
    ~MonitorService();

    void start(const std::vector<std::string>& paths);
    void stop();

   private:
    void run();

    DetectorService& detector_;
    const ConfigManager& config_;
    IReportObserver& observer_;

    int fan_fd_ = -1;
    int stop_fd_ = -1;
    std::atomic<bool> is_running_{false};
    std::thread worker_thread_;
};

}  // namespace warden::services
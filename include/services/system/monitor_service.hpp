#pragma once
#include <atomic>
#include <string>
#include <thread>
#include <vector>

#include "services/common/report_observer.hpp"

namespace warden::services {

class DetectorService;
class ConfigService;

class MonitorService {
   public:
    MonitorService(DetectorService& ds, ConfigService& cs, IReportObserver& observer);
    ~MonitorService();

    void start(const std::vector<std::string>& paths);
    void stop();

   private:
    void run();

    DetectorService& detector_;
    ConfigService& config_;
    IReportObserver& observer_;

    int fan_fd_ = -1;
    int stop_fd_ = -1;
    std::atomic<bool> is_running_{false};
    std::thread worker_thread_;
};

}  // namespace warden::services
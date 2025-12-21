#pragma once

#include <atomic>
#include <map>
#include <string>
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
    void process_events();

    DetectorService& detector_;
    ConfigService& config_;
    IReportObserver& observer_;

    int inotify_fd_ = -1;
    std::atomic<bool> is_running_{false};
    std::map<int, std::string> watch_descriptors_;
};

}  // namespace warden::services
#pragma once
#include "common/logger.hpp"
#include "common/report_observer.hpp"

namespace warden::common {

class LogObserver : public warden::services::IReportObserver {
   public:
    void notify_detection(const std::string& path,
                          const warden::services::DetectionResult& result) override {
        Logger::log_detection(path, result);
    }
};

}  // namespace warden::common
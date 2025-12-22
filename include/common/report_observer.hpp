#pragma once
#include <string>

namespace warden::services {

struct DetectionResult;

class IReportObserver {
   public:
    virtual ~IReportObserver() = default;
    virtual void notify_detection(const std::string& path, const DetectionResult& result) = 0;
};

}  // namespace warden::services
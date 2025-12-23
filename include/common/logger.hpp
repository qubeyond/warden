#pragma once

#include <string>

namespace warden::services {
struct DetectionResult;
struct LoggerConfig;
}  // namespace warden::services

namespace warden::common {

class Logger {
   public:
    static void init(const warden::services::LoggerConfig& cfg);
    static void log_detection(const std::string& path,
                              const warden::services::DetectionResult& result);
};

}  // namespace warden::common
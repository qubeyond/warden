#pragma once
#include <iomanip>
#include <string>
#include <vector>

#include "services/common/report_observer.hpp"

namespace warden::services {

struct CliOptions {
    std::string file_path;
    bool verbose = false;
    float custom_threshold = -1.0f;
};

class CliService : public IReportObserver {
   public:
    CliService() = default;

    bool parse(int argc, char** argv, CliOptions& options);

    void notify_detection(const std::string& path, const DetectionResult& result) override {
        print_report(path, result);
    }

    void print_report(const std::string& path, const struct DetectionResult& result);
};

}  // namespace warden::services
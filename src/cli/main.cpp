#include <atomic>
#include <csignal>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

#include "common/config.hpp"
#include "common/logger.hpp"
#include "services/cli/cli_service.hpp"
#include "services/core/detector_service.hpp"
#include "services/core/feature_service.hpp"
#include "services/core/model_service.hpp"
#include "services/core/scan_service.hpp"
#include "services/system/monitor_service.hpp"

using namespace warden::services;

std::atomic<bool> keep_running{true};

void signal_handler(int) {
    keep_running = false;
}

class CliObserver : public IReportObserver {
    CliService& cli_;

   public:
    explicit CliObserver(CliService& cli) : cli_(cli) {
    }
    void notify_detection(const std::string& path, const DetectionResult& result) override {
        cli_.print_report(path, result);
        warden::common::Logger::log_detection(path, result);
    }
};

int main(int argc, char** argv) {
    CliService cli;
    CliOptions options;

    if (!cli.parse(argc, argv, options)) return 0;

    try {
        auto config = ConfigManager::load("configs/app_config.json", 
                                         "configs/model_config_v2.json",
                                         "configs/properties.json");

        warden::common::Logger::init(config->logger());

        ScanService scanner(*config);
        FeatureService extractor(*config);
        ModelService model(*config);
        DetectorService detector(scanner, extractor, model);

        float threshold =
            (options.custom_threshold > 0) ? options.custom_threshold : config->model().threshold;

        if (options.mode_monitor) {
            CliObserver observer(cli);
            MonitorService monitor(detector, *config, observer);
            std::signal(SIGINT, signal_handler);
            std::cout << "\033[1;34m[*] Warden AI Real-time Monitor Active\033[0m" << std::endl;
  
            std::string path_to_watch = options.monitor_path.empty() ? 
                                        (config->scanner().watch_dirs.empty() ? "." : config->scanner().watch_dirs[0]) : 
                                        options.monitor_path;

            monitor.start({path_to_watch});
            
            while (keep_running) {
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }
            monitor.stop();
        } else {
            if (options.file_path.empty()) return 1;
            auto result = detector.process_file(options.file_path, threshold);
            cli.print_report(options.file_path, result);
            warden::common::Logger::log_detection(options.file_path, result);
        }
    } catch (const std::exception& e) {
        std::cerr << "[!] Fatal Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
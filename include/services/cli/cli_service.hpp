#pragma once
#include <string>
#include <vector>
#include <deque>
#include <optional>
#include <memory>
#include <atomic>
#include <mutex>
#include <functional>

#include "common/report_observer.hpp"

namespace ftxui { class ScreenInteractive; }

namespace warden::services {

class DetectorService;
struct DetectionResult;
class ConfigManager;

struct CliOptions {
    std::string file_path;
    std::string monitor_path;
    std::string app_config_path;
    std::string model_config_path;
    std::string prop_config_path;

    float custom_threshold = -1.0f;
    bool mode_monitor = false;
    bool verbose = false;
    bool show_gui = false;        
    bool save_config = false;

    std::optional<std::string> set_log_level;
    std::optional<size_t> set_max_files;
    std::optional<size_t> set_max_chunks;
};

struct GuiState {
    std::atomic<int> scanned_count{0};
    std::atomic<int> threats_count{0};
    std::deque<std::string> event_log; 
    std::mutex log_mutex;
};

class CliService : public IReportObserver {
public:
    CliService();
    ~CliService();

    int run(int argc, char** argv);

    void notify_detection(const std::string& path, const DetectionResult& result) override;

    void print_report(const std::string& path, const struct DetectionResult& result);
    void print_message(const std::string& msg, const std::string& color = "");
    void print_error(const std::string& msg);

private:
    bool parse(int argc, char** argv, CliOptions& options);
    void apply_overrides(std::shared_ptr<ConfigManager> cfg, const CliOptions& opts);
    
    void run_gui_mode(std::shared_ptr<ConfigManager> config, DetectorService& detector, const CliOptions& opts);

    GuiState gui_state_;
    ftxui::ScreenInteractive* active_screen_ = nullptr; 
};

} // namespace warden::services
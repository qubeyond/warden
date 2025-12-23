#include "services/cli/cli_service.hpp"
#include <CLI/App.hpp>
#include <CLI/Config.hpp>
#include <CLI/Formatter.hpp>
#include <iostream>
#include <thread>
#include <fstream>
#include <iomanip>
#include <sys/fanotify.h>
#include <nlohmann/json.hpp>

#include "services/core/scan_service.hpp"
#include "services/core/feature_service.hpp"
#include "services/core/model_service.hpp"
#include "services/core/detector_service.hpp"
#include "services/system/monitor_service.hpp"
#include "common/logger.hpp"
#include "common/config.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

namespace warden::services {

CliService::CliService() = default;
CliService::~CliService() = default;

bool CliService::parse(int argc, char** argv, CliOptions& options) {
    CLI::App app{"Warden AI"};
    app.fallthrough();

    app.add_option("-a,--app-config", options.app_config_path, "App config")->default_val("configs/app_config.json");
    app.add_option("-m,--model-config", options.model_config_path, "Model config")->default_val("configs/model_config_v2.json");
    app.add_option("-p,--prop-config", options.prop_config_path, "Properties path")->default_val("configs/properties.json");

    app.add_flag("--gui", options.show_gui);
    app.add_flag("--save", options.save_config);

    app.add_option("-t,--set-threshold", options.custom_threshold);
    app.add_option("--set-log-level", options.set_log_level);
    app.add_option("--set-max-chunks", options.set_max_chunks);

    auto scan_cmd = app.add_subcommand("scan");
    scan_cmd->add_option("target", options.file_path);

    auto monitor_cmd = app.add_subcommand("monitor");
    monitor_cmd->add_option("path", options.monitor_path);

    try {
        app.parse(argc, argv);
        options.mode_monitor = monitor_cmd->parsed();
    } catch (const CLI::ParseError& e) {
        app.exit(e);
        return false;
    }
    return true;
}

void CliService::apply_overrides(std::shared_ptr<ConfigManager> cfg, const CliOptions& opts) {
    if (opts.custom_threshold >= 0) cfg->mutable_model().threshold = opts.custom_threshold;
    if (opts.set_log_level) cfg->mutable_logger().log_level = *opts.set_log_level;
    if (opts.set_max_chunks) cfg->mutable_scanner().max_chunks = *opts.set_max_chunks;
}

void CliService::run_gui_mode(std::shared_ptr<ConfigManager> config, DetectorService& detector, const CliOptions& opts) {
    using namespace ftxui;

    std::string watch_path = opts.monitor_path.empty() ? 
        (config->scanner().watch_dirs.empty() ? "." : config->scanner().watch_dirs[0]) : 
        opts.monitor_path;

    std::string log_file = config->logger().log_dir + "/warden.json";
    std::ifstream ifs(log_file);
    if (ifs.is_open()) {
        std::string line;
        std::lock_guard<std::mutex> lock(gui_state_.log_mutex);
        while (std::getline(ifs, line)) {
            try {
                auto j = nlohmann::json::parse(line);
                std::string v = j["verdict"];
                std::string p = j["path"];
                float prob = j["prob"];
                std::stringstream ss;
                ss << (v == "MALICIOUS" ? "[!] " : "[+] ") << v << " | " << p << " (" << (int)(prob * 100) << "%)";
                gui_state_.scanned_count++;
                if (v == "MALICIOUS") gui_state_.threats_count++;
                gui_state_.event_log.push_front(ss.str());
                if (gui_state_.event_log.size() > 50) gui_state_.event_log.pop_back();
            } catch (...) { continue; }
        }
    }

    std::thread monitor_thread([&, watch_path]() {
        MonitorService monitor(detector, *config, *this);
        monitor.start({watch_path});
        while(true) std::this_thread::sleep_for(std::chrono::seconds(1));
    });
    monitor_thread.detach();

    auto screen = ScreenInteractive::Fullscreen();
    active_screen_ = &screen;

    auto btn_quit = Button("QUIT", [&] { screen.Exit(); }, ButtonOption::Animated(Color::Red));

    auto renderer = Renderer(Container::Vertical({btn_quit}), [&] {
        std::vector<Element> log_elements;
        {
            std::lock_guard<std::mutex> lock(gui_state_.log_mutex);
            for (const auto& line : gui_state_.event_log) {
                if (line.find("MALICIOUS") != std::string::npos) 
                    log_elements.push_back(text(line) | color(Color::RedLight));
                else if (line.find("SUSPICIOUS") != std::string::npos)
                    log_elements.push_back(text(line) | color(Color::Yellow));
                else 
                    log_elements.push_back(text(line) | color(Color::Green));
            }
        }

        auto stats_box = vbox({
            text(" WARDEN AI MONITOR ") | bold | center | color(Color::Cyan),
            separator(),
            hbox({ text(" Target: ") | color(Color::Yellow), text(watch_path) }),
            hbox({ text(" Threshold: ") | color(Color::Yellow), text(std::to_string(config->model().threshold)) }),
            separator(),
            hbox({ text(" Scanned: ") | color(Color::Blue), text(std::to_string(gui_state_.scanned_count)) | bold }),
            hbox({ text(" Threats: ") | color(Color::Red), text(std::to_string(gui_state_.threats_count)) | bold | blink }),
        }) | border | flex;

        auto log_box = vbox(std::move(log_elements)) | border | flex;

        return vbox({
            hbox({ stats_box, log_box }) | flex,
            separator(),
            hbox({ text(" Status: ACTIVE ") | color(Color::Green), filler(), btn_quit->Render() })
        }) | border;
    });

    screen.Loop(renderer);
    active_screen_ = nullptr;
}

int CliService::run(int argc, char** argv) {
    CliOptions opts;
    if (!parse(argc, argv, opts)) return 0;

    try {
        auto config = ConfigManager::load(opts.app_config_path, opts.model_config_path, opts.prop_config_path);
        apply_overrides(config, opts);
        warden::common::Logger::init(config->logger());

        ScanService scanner(*config);
        FeatureService extractor(*config);
        ModelService model(*config);
        DetectorService detector(scanner, extractor, model);

        if (opts.show_gui) {
            run_gui_mode(config, detector, opts); 
            return 0;
        }

        if (opts.mode_monitor) {
            std::string path = opts.monitor_path.empty() ? 
                (config->scanner().watch_dirs.empty() ? "." : config->scanner().watch_dirs[0]) : opts.monitor_path;
            
            print_message("[*] Warden CLI Monitor: " + path, "\033[1;34m");
            MonitorService monitor(detector, *config, *this);
            monitor.start({path});
            while (true) std::this_thread::sleep_for(std::chrono::milliseconds(200));
        } else if (!opts.file_path.empty()) {
            auto res = detector.process_file(opts.file_path, config->model().threshold);
            print_report(opts.file_path, res);
            warden::common::Logger::log_detection(opts.file_path, res);
        }

    } catch (const std::exception& e) {
        print_error(e.what());
        return 1;
    }
    return 0;
}

void CliService::notify_detection(const std::string& path, const DetectionResult& result) {
    if (!active_screen_) {
        print_report(path, result);
        warden::common::Logger::log_detection(path, result);
        return;
    }

    gui_state_.scanned_count++;
    
    std::string v_str = (result.verdict == warden::common::Verdict::MALICIOUS) ? "MALICIOUS" : 
                        (result.verdict == warden::common::Verdict::SUSPICIOUS ? "SUSPICIOUS" : "SAFE");
    
    if (result.verdict == warden::common::Verdict::MALICIOUS) {
        gui_state_.threats_count++;
    }

    std::stringstream ss;
    ss << (result.verdict == warden::common::Verdict::SAFE ? "[+] " : "[!] ") 
       << v_str << " | " << path << " (" << (int)(result.max_probability * 100) << "%)";
    
    {
        std::lock_guard<std::mutex> lock(gui_state_.log_mutex);
        gui_state_.event_log.push_front(ss.str());
        if (gui_state_.event_log.size() > 50) gui_state_.event_log.pop_back();
    }

    if (active_screen_) {
        active_screen_->PostEvent(ftxui::Event::Custom);
    }
    
    warden::common::Logger::log_detection(path, result);
}

void CliService::print_message(const std::string& msg, const std::string& color) {
    std::cout << color << msg << "\033[0m" << std::endl;
}

void CliService::print_error(const std::string& msg) {
    std::cerr << "\033[1;31m[!] Error: " << msg << "\033[0m" << std::endl;
}

void CliService::print_report(const std::string& path, const DetectionResult& result) {
    const std::string R = "\033[1;31m", G = "\033[1;32m", Y = "\033[1;33m", B = "\033[1;34m", RESET = "\033[0m";
    std::string v_str = (result.verdict == warden::common::Verdict::MALICIOUS) ? "[!] MALICIOUS" : 
                        (result.verdict == warden::common::Verdict::SUSPICIOUS ? "[?] SUSPICIOUS" : "[+] SAFE");
    std::string v_col = (result.verdict == warden::common::Verdict::MALICIOUS) ? R : 
                        (result.verdict == warden::common::Verdict::SUSPICIOUS ? Y : G);

    std::cout << v_col << std::left << std::setw(14) << v_str << RESET << " | " 
              << B << std::fixed << std::setprecision(1) << std::setw(5) << (result.max_probability * 100.0f) << "%" << RESET 
              << " | " << path << std::endl;
}

}
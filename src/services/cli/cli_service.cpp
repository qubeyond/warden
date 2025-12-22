#include "services/cli/cli_service.hpp"

#include <sys/fanotify.h>

#include <CLI/App.hpp>
#include <CLI/Config.hpp>
#include <CLI/Formatter.hpp>
#include <iomanip>
#include <iostream>

#include "services/core/detector_service.hpp"

namespace warden::services {

bool CliService::parse(int argc, char** argv, CliOptions& options) {
    CLI::App app{"Warden AI - Machine Learning MALICIOUS Detector"};

    auto scan_cmd = app.add_subcommand("scan", "Scan a single file");
    scan_cmd->add_option("file", options.file_path, "Path to file")
        ->required()
        ->check(CLI::ExistingFile);

    auto monitor_cmd = app.add_subcommand("monitor", "Monitor directory in real-time");
    monitor_cmd->add_option("path", options.monitor_path, "Directory to watch")
        ->required()
        ->check(CLI::ExistingDirectory);

    app.add_flag("-v,--verbose", options.verbose, "Enable detailed info");
    app.add_option("-t,--threshold", options.custom_threshold, "Override threshold (0.0 - 1.0)");

    try {
        app.parse(argc, argv);
        options.mode_monitor = (monitor_cmd && monitor_cmd->parsed());
    } catch (const CLI::ParseError& e) {
        app.exit(e);
        return false;
    }
    return true;
}

void CliService::print_report(const std::string& path, const DetectionResult& result) {
    const std::string R = "\033[1;31m";
    const std::string G = "\033[1;32m";
    const std::string Y = "\033[1;33m";
    const std::string B = "\033[1;34m";
    const std::string RESET = "\033[0m";

    std::string ev_str = "UNKNOWN";
    if (result.event_mask & FAN_CLOSE_WRITE)
        ev_str = "FINISHED";
    else if (result.event_mask & FAN_MODIFY)
        ev_str = "MODIFY";
    else if (result.event_mask & FAN_ACCESS)
        ev_str = "READ";
    else if (result.event_mask & FAN_OPEN)
        ev_str = "OPEN";

    std::string v_str;
    std::string v_color = RESET;
    if (result.verdict == warden::common::Verdict::MALICIOUS) {
        v_str = "[!] MALICIOUS";
        v_color = R;
    } else if (result.verdict == warden::common::Verdict::SUSPICIOUS) {
        v_str = "[?] SUSPICIOUS";
        v_color = Y;
    } else {
        v_str = "[+] SAFE";
        v_color = G;
    }

    std::string t_str;
    switch (result.file_type) {
        case warden::common::FileType::MEDIA:
            t_str = "MEDIA";
            break;
        case warden::common::FileType::ARCHIVE:
            t_str = "ARCH";
            break;
        case warden::common::FileType::EXECUTABLE:
            t_str = "EXEC";
            break;
        default:
            t_str = "OTHER";
            break;
    }

    std::cout << v_color << std::left << std::setw(14) << v_str << RESET << " | " << B << std::left
              << std::setw(8) << ev_str << RESET << " | " << std::left << std::setw(6) << t_str
              << " | " << std::right << std::fixed << std::setprecision(1) << std::setw(5)
              << (result.max_probability * 100.0f) << "% | " << std::right << std::setw(3)
              << result.suspicious_chunks << "/" << std::left << std::setw(3) << result.total_chunks
              << " | " << path << std::endl;
}

}  // namespace warden::services
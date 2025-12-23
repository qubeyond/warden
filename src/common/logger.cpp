#include "common/logger.hpp"

#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/spdlog.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

#include "common/config.hpp"
#include "services/core/detector_service.hpp"

namespace fs = std::filesystem;

namespace warden::common {

static std::string get_valid_log_path(const std::string& dir) {
    try {
        if (!fs::exists(dir)) fs::create_directories(dir);
        fs::path test_file = fs::path(dir) / ".perm_test";
        std::ofstream(test_file).close();
        fs::remove(test_file);
        return (fs::path(dir) / "warden.json").string();
    } catch (...) {
        return "warden.json";
    }
}

void Logger::init(const warden::services::LoggerConfig& cfg) {
    std::string path = get_valid_log_path(cfg.log_dir);

    auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
        path, 1024 * 1024 * cfg.max_file_size_mb, cfg.max_files);

    file_sink->set_pattern("%v");
    auto logger = std::make_shared<spdlog::logger>("warden", file_sink);
    spdlog::set_default_logger(logger);

    if (cfg.log_level == "all")
        spdlog::set_level(spdlog::level::trace);
    else if (cfg.log_level == "warning")
        spdlog::set_level(spdlog::level::warn);
    else if (cfg.log_level == "error")
        spdlog::set_level(spdlog::level::err);
    else
        spdlog::set_level(spdlog::level::info);

    spdlog::flush_on(spdlog::level::info);
}

void Logger::log_detection(const std::string& path,
                           const warden::services::DetectionResult& result) {
    auto lvl = spdlog::get_level();
    if (lvl >= spdlog::level::warn && result.verdict == warden::common::Verdict::SAFE) return;

    nlohmann::json j;
    j["t"] = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    j["path"] = path;
    j["verdict"] = (result.verdict == warden::common::Verdict::MALICIOUS)    ? "MALICIOUS"
                   : (result.verdict == warden::common::Verdict::SUSPICIOUS) ? "SUSPICIOUS"
                                                                             : "SAFE";
    j["prob"] = result.max_probability;
    j["suspicious"] = result.suspicious_chunks;
    j["total"] = result.total_chunks;

    spdlog::info(j.dump());
}

}  // namespace warden::common
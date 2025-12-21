#include "services/cli/cli_service.hpp"

#include <CLI/App.hpp>
#include <CLI/Config.hpp>
#include <CLI/Formatter.hpp>
#include <iostream>

#include "services/core/detector_service.hpp"

namespace warden::services {

bool CliService::parse(int argc, char** argv, CliOptions& options) {
    CLI::App app{"Warden AI - Machine Learning Malware Detector"};

    app.add_option("file", options.file_path, "Path to the file to scan")
        ->required()
        ->check(CLI::ExistingFile);

    app.add_flag("-v,--verbose", options.verbose, "Enable detailed chunk information");

    app.add_option("-t,--threshold", options.custom_threshold,
                   "Override detection threshold (0.0 - 1.0)");

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError& e) {
        app.exit(e);
        return false;
    }
    return true;
}

void CliService::print_report(const std::string& path, const DetectionResult& result) {
    std::string type_str;
    switch (result.file_type) {
        case warden::common::FileType::MEDIA:
            type_str = "MEDIA";
            break;
        case warden::common::FileType::ARCHIVE:
            type_str = "ARCHIVE";
            break;
        case warden::common::FileType::EXECUTABLE:
            type_str = "EXEC";
            break;
        default:
            type_str = "OTHER";
            break;
    }

    std::string v_str;
    if (result.verdict == warden::common::Verdict::MALWARE)
        v_str = "[!] MALWARE";
    else if (result.verdict == warden::common::Verdict::SUSPICIOUS)
        v_str = "[?] SUSPICIOUS";
    else
        v_str = "[+] SAFE";

    std::cout << std::left << std::setw(14) << v_str << " | " << std::left << std::setw(8)
              << type_str << " | " << std::right << std::fixed << std::setprecision(2)
              << std::setw(6) << (result.max_probability * 100.0f) << "% | " << std::right
              << std::setw(3) << result.suspicious_chunks << "/" << std::left << std::setw(7)
              << std::to_string(result.total_chunks) + " chunks" << " | " << path << std::endl;
}

}  // namespace warden::services
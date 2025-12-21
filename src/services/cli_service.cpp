#include "services/cli_service.hpp"
#include "services/detector_service.hpp"

#include <CLI/App.hpp>
#include <CLI/Config.hpp>
#include <CLI/Formatter.hpp>
#include <iostream>

namespace warden::services {

bool CliService::parse(int argc, char** argv, CliOptions& options) {
    CLI::App app{"Warden AI - Machine Learning Malware Detector"};

    app.add_option("file", options.file_path, "Path to the file to scan")
       ->required()
       ->check(CLI::ExistingFile);

    app.add_flag("-v,--verbose", options.verbose, "Enable detailed chunk information");

    app.add_option("-t,--threshold", options.custom_threshold, "Override detection threshold (0.0 - 1.0)");

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError &e) {
        app.exit(e); 
        return false;
    }
    return true;
}

void CliService::print_report(const std::string& path, const DetectionResult& result) {
    std::cout << "\n==========================================" << std::endl;
    std::cout << " WARDEN ANALYSIS REPORT " << std::endl;
    std::cout << "==========================================" << std::endl;
    std::cout << " Target: " << path << std::endl;
    
    std::string verdict_str;
    if (result.verdict == warden::common::Verdict::MALWARE) verdict_str = "[!] MALWARE";
    else if (result.verdict == warden::common::Verdict::SUSPICIOUS) verdict_str = "[?] SUSPICIOUS";
    else verdict_str = "[+] SAFE";

    std::cout << " Verdict: " << verdict_str << std::endl;
    std::cout << " Confidence: " << (result.max_probability * 100.0f) << "%" << std::endl;
    std::cout << " Chunks: " << result.suspicious_chunks << "/" << result.total_chunks << " suspicious" << std::endl;
    std::cout << "==========================================\n" << std::endl;
}

}
#pragma once
#include <string>
#include <vector>

namespace warden::services {

struct CliOptions {
    std::string file_path;
    bool verbose = false;
    float custom_threshold = -1.0f; 
};

class CliService {
public:
    CliService() = default;
    
    bool parse(int argc, char** argv, CliOptions& options);
    void print_report(const std::string& path, const struct DetectionResult& result);
};

}
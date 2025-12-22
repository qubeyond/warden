#pragma once

#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace warden::services {

struct AppConfig {
    std::string db_host = "localhost";
    uint16_t db_port = 8123;
};

struct ModelConfig {
    std::string path;
    float threshold = 0.5f;
    size_t n_features = 262;
};

struct ScannerConfig {
    size_t min_chunks = 10;
    size_t max_chunks = 50;
    std::vector<std::string> watch_dirs;
};

class ConfigService {
   public:
    static std::shared_ptr<ConfigService> load(const std::string& app_path,
                                               const std::string& model_path,
                                               const std::string& prop_path);

    const AppConfig& app() const {
        return app_;
    }
    const ModelConfig& model() const {
        return model_;
    }
    const ScannerConfig& scanner() const {
        return scanner_;
    }

   private:
    ConfigService() = default;

    AppConfig app_;
    ModelConfig model_;
    ScannerConfig scanner_;

    static nlohmann::json parse_file(const std::string& path);
};

}  // namespace warden::services
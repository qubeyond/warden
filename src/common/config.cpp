#include "common/config.hpp"

#include <fstream>
#include <stdexcept>

namespace warden::services {

nlohmann::json ConfigManager::parse_file(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) throw std::runtime_error("File not found: " + path);
    return nlohmann::json::parse(f);
}

std::shared_ptr<ConfigManager> ConfigManager::load(const std::string& app_path,
                                                   const std::string& model_path,
                                                   const std::string& prop_path) {
    auto cfg = std::shared_ptr<ConfigManager>(new ConfigManager());

    auto j_app = parse_file(app_path);
    if (j_app.contains("database")) {
        cfg->app_.db_host = j_app["database"].value("host", "localhost");
        cfg->app_.db_port = j_app["database"].value("port", 8123);
    }

    auto j_model = parse_file(model_path);
    cfg->model_.path = j_model.at("model_file").get<std::string>();
    cfg->model_.threshold = j_model.value("threshold", 0.5f);
    cfg->model_.n_features = j_model.value("n_features", 262);

    auto j_prop = parse_file(prop_path);
    if (j_prop.contains("scanner")) {
        auto& s = j_prop["scanner"];
        cfg->scanner_.min_chunks = s.value("min_chunks", 10);
        cfg->scanner_.max_chunks = s.value("max_chunks", 50);
        cfg->scanner_.watch_dirs = s.value("watch_dirs", std::vector<std::string>{});
    }

    if (j_prop.contains("logger")) {
        auto& l = j_prop["logger"];
        cfg->logger_.log_dir = l.value("log_dir", "/var/log/warden");
        cfg->logger_.log_level = l.value("log_level", "info");
        cfg->logger_.max_file_size_mb = l.value("max_file_size_mb", 10);
        cfg->logger_.max_files = l.value("max_files", 3);
    }

    return cfg;
}

}  // namespace warden::services
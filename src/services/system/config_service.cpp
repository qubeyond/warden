#include "services/system/config_service.hpp"

#include <fstream>
#include <stdexcept>

namespace warden::services {

nlohmann::json ConfigService::parse_file(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) {
        throw std::runtime_error("Critical: Configuration file not found: " + path);
    }
    try {
        return nlohmann::json::parse(f);
    } catch (const nlohmann::json::parse_error& e) {
        throw std::runtime_error("Critical: JSON parse error in " + path + ": " + e.what());
    }
}

std::shared_ptr<ConfigService> ConfigService::load(const std::string& app_path,
                                                   const std::string& model_path,
                                                   const std::string& prop_path) {
    auto cfg = std::shared_ptr<ConfigService>(new ConfigService());

    // App Config
    auto j_app = parse_file(app_path);
    if (j_app.contains("database")) {
        cfg->app_.db_host = j_app["database"].value("host", "localhost");
        cfg->app_.db_port = j_app["database"].value("port", 8123);
    }

    // Model Config
    auto j_model = parse_file(model_path);
    cfg->model_.path = j_model.at("model_file").get<std::string>();
    cfg->model_.threshold = j_model.value("threshold", 0.5f);
    cfg->model_.n_features = j_model.value("n_features", 262);

    // Properties Config
    auto j_prop = parse_file(prop_path);
    if (j_prop.contains("scanner")) {
        auto& s = j_prop["scanner"];
        cfg->scanner_.min_chunks = s.value("min_chunks", 10);
        cfg->scanner_.max_chunks = s.value("max_chunks", 50);
        cfg->scanner_.watch_dirs = s.value("watch_dirs", std::vector<std::string>{});
    }

    return cfg;
}

}  // namespace warden::services
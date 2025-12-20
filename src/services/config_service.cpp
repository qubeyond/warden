#include "services/config_service.hpp"

#include <fstream>
#include <stdexcept>

namespace warden::services {

ConfigService::ConfigService(const std::string &app_cfg_path, const std::string &model_cfg_path) {
    app_data_ = load_file(app_cfg_path);
    model_data_ = load_file(model_cfg_path);
}

nlohmann::json ConfigService::load_file(const std::string &path) {
    std::ifstream f(path);
    if (!f.is_open()) throw std::runtime_error("Config not found: " + path);
    return nlohmann::json::parse(f);
}

}  // namespace warden::services
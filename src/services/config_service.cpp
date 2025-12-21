#include "services/config_service.hpp"
#include <fstream>
#include <stdexcept>

namespace warden::services {

ConfigService::ConfigService(const std::string &app_path, 
                          const std::string &model_path, 
                          const std::string &prop_path) {
    app_data_ = load_file(app_path);
    model_data_ = load_file(model_path);
    prop_data_ = load_file(prop_path);
}

nlohmann::json ConfigService::load_file(const std::string &path) {
    std::ifstream f(path);
    if (!f.is_open()) throw std::runtime_error("Config not found: " + path);
    return nlohmann::json::parse(f);
}

}
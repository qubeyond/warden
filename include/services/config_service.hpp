#pragma once
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include "common/defs.hpp"

namespace warden::services {

class ConfigService {
   public:
    explicit ConfigService(const std::string &app_cfg_path, const std::string &model_cfg_path);

    float get_threshold() const {
        return model_data_.at("threshold");
    }
    size_t get_features() const {
        return model_data_.at("n_features");
    }
    std::string get_model_path() const {
        return model_data_.at("model_file");
    }
    std::string get_model_type() const {
        return model_data_.at("model_type");
    }

    std::string get_db_host() const {
        return app_data_.at("database").value("host", "localhost");
    }
    std::vector<std::string> get_watch_dirs() const {
        return app_data_.at("scanner").value("watch_dirs", std::vector<std::string>{});
    }

   private:
    nlohmann::json model_data_;
    nlohmann::json app_data_;
    nlohmann::json load_file(const std::string &path);
};

}  // namespace warden::services
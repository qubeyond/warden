#pragma once
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include "common/defs.hpp"

namespace warden::services {

class ConfigService {
   public:
    explicit ConfigService(const std::string &app_path, const std::string &model_path,
                           const std::string &prop_path);

    float get_threshold() const {
        return model_data_.at("threshold");
    }
    size_t get_features() const {
        return model_data_.at("n_features");
    }
    std::string get_model_path() const {
        return model_data_.at("model_file");
    }

    std::string get_db_host() const {
        return app_data_.at("database").value("host", "localhost");
    }

    size_t get_min_chunks() const {
        return prop_data_.at("scanner").value("min_chunks", 10);
    }
    size_t get_max_chunks() const {
        return prop_data_.at("scanner").value("max_chunks", 50);
    }
    std::vector<std::string> get_watch_dirs() const {
        return prop_data_.at("scanner").value("watch_dirs", std::vector<std::string>{});
    }

   private:
    nlohmann::json model_data_;
    nlohmann::json app_data_;
    nlohmann::json prop_data_;
    nlohmann::json load_file(const std::string &path);
};

}  // namespace warden::services
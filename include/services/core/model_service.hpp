#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace warden::services {
class ConfigManager;
}

namespace warden::services {

class ModelService {
   public:
    explicit ModelService(const ConfigManager& config);
    ~ModelService();

    ModelService(const ModelService&) = delete;
    ModelService& operator=(const ModelService&) = delete;

    float predict(const std::vector<float>& features);

   private:
    const ConfigManager& config_;
    void* booster_;
    mutable std::mutex model_mutex_;

    void check_xgboost_error(int code, const std::string& msg) const;
};

}  // namespace warden::services
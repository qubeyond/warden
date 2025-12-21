#pragma once
#include <xgboost/c_api.h>
#include <string>
#include <vector>
#include "services/config_service.hpp"

namespace warden::services {

class ModelService {
public:
    explicit ModelService(const ConfigService& config);
    ~ModelService();

    float predict(const std::vector<float>& features);

private:
    const ConfigService& config_;
    BoosterHandle booster_;
};

}
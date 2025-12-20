#pragma once
#include <xgboost/c_api.h>

#include <string>
#include <vector>

#include "common/defs.hpp"

namespace warden::services {

class ModelService {
   public:
    explicit ModelService(const std::string& model_path);
    ~ModelService();

    float predict(const std::vector<float>& features);

   private:
    BoosterHandle booster_;
};

}  // namespace warden::services
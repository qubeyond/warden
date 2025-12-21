#include "services/core/model_service.hpp"

#include <stdexcept>

namespace warden::services {

ModelService::ModelService(const ConfigService& config) : config_(config), booster_(nullptr) {
    std::string model_path = config_.get_model_path();

    if (XGBoosterCreate(nullptr, 0, &booster_) != 0) {
        throw std::runtime_error("Failed to create XGBoost booster");
    }

    if (XGBoosterLoadModel(booster_, model_path.c_str()) != 0) {
        throw std::runtime_error("Failed to load model: " + model_path);
    }
}

ModelService::~ModelService() {
    if (booster_) {
        XGBoosterFree(booster_);
    }
}

float ModelService::predict(const std::vector<float>& features) {
    if (features.empty()) return 0.0f;

    size_t expected = config_.get_features();
    if (features.size() != expected) {
        throw std::runtime_error("Model input mismatch: expected " + std::to_string(expected) +
                                 ", got " + std::to_string(features.size()));
    }

    DMatrixHandle dmat;
    if (XGDMatrixCreateFromMat(features.data(), 1, features.size(), -1.0f, &dmat) != 0) {
        throw std::runtime_error("Failed to create DMatrix");
    }

    bst_ulong out_len;
    const float* out_result;

    int ret = XGBoosterPredict(booster_, dmat, 0, 0, 0, &out_len, &out_result);
    XGDMatrixFree(dmat);

    if (ret != 0) {
        throw std::runtime_error("XGBoost prediction failed");
    }

    return (out_len > 0) ? out_result[0] : 0.0f;
}

}  // namespace warden::services
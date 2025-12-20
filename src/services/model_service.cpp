#include "services/model_service.hpp"

#include <stdexcept>

namespace warden::services {

ModelService::ModelService(const std::string& model_path) : booster_(nullptr) {
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

    DMatrixHandle dmat;
    XGDMatrixCreateFromMat(features.data(), 1, features.size(), -1.0f, &dmat);

    bst_ulong out_len;
    const float* out_result;

    if (XGBoosterPredict(booster_, dmat, 0, 0, 0, &out_len, &out_result) != 0) {
        XGDMatrixFree(dmat);
        throw std::runtime_error("XGBoost prediction failed");
    }

    float score = (out_len > 0) ? out_result[0] : 0.0f;
    XGDMatrixFree(dmat);

    return score;
}

}  // namespace warden::services
#include "services/core/model_service.hpp"

#include <xgboost/c_api.h>

#include <stdexcept>

#include "services/system/config_service.hpp"

namespace warden::services {

struct DMatrixGuard {
    DMatrixHandle handle;
    explicit DMatrixGuard(DMatrixHandle h) : handle(h) {
    }
    ~DMatrixGuard() {
        if (handle) XGDMatrixFree(handle);
    }
};

ModelService::ModelService(const ConfigService& config) : config_(config), booster_(nullptr) {
    std::string model_path = config_.model().path;
    BoosterHandle bh;

    check_xgboost_error(XGBoosterCreate(nullptr, 0, &bh), "Failed to create booster");
    booster_ = bh;

    if (XGBoosterLoadModel(bh, model_path.c_str()) != 0) {
        XGBoosterFree(bh);
        throw std::runtime_error("XGBoost: Unable to load model from " + model_path);
    }
}

ModelService::~ModelService() {
    if (booster_) XGBoosterFree(static_cast<BoosterHandle>(booster_));
}

float ModelService::predict(const std::vector<float>& features) {
    if (features.empty()) return 0.0f;
    if (features.size() != config_.model().n_features) {
        throw std::runtime_error("Model input mismatch");
    }

    DMatrixHandle dmat_raw;
    check_xgboost_error(
        XGDMatrixCreateFromMat(features.data(), 1, features.size(), -1.0f, &dmat_raw),
        "Failed to create DMatrix");
    DMatrixGuard dmat(dmat_raw);

    bst_ulong out_len;
    const float* out_result;
    {
        std::lock_guard<std::mutex> lock(model_mutex_);
        check_xgboost_error(XGBoosterPredict(static_cast<BoosterHandle>(booster_), dmat.handle, 0,
                                             0, 0, &out_len, &out_result),
                            "Prediction failed");
    }
    return (out_len > 0) ? out_result[0] : 0.0f;
}

void ModelService::check_xgboost_error(int code, const std::string& msg) const {
    if (code != 0) {
        const char* err = XGBGetLastError();
        throw std::runtime_error("XGBoost Error [" + msg + "]: " + (err ? err : "unknown"));
    }
}

}  // namespace warden::services
#pragma once
#include <string>
#include <vector>

#include "common/defs.hpp"
#include "services/feature_service.hpp"
#include "services/model_service.hpp"
#include "services/scan_service.hpp"

namespace warden::services {

struct DetectionResult {
    warden::common::Verdict verdict;
    float max_probability;
    size_t suspicious_chunks;
    size_t total_chunks;
};

class DetectorService {
   public:
    DetectorService(ScanService& ss, FeatureService& fs, ModelService& ms);
    DetectionResult process_file(const std::string& path, float threshold);

   private:
    ScanService& scan_service_;
    FeatureService& feature_service_;
    ModelService& model_service_;
};

}  // namespace warden::services
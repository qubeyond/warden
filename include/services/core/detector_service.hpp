#pragma once
#include <string>
#include <vector>

#include "common/defs.hpp"
#include "services/core/feature_service.hpp"
#include "services/core/model_service.hpp"
#include "services/core/scan_service.hpp"

namespace warden::services {

struct DetectionResult {
    warden::common::Verdict verdict;
    float max_probability;
    size_t suspicious_chunks;
    size_t total_chunks;
    warden::common::FileType file_type;
    uint64_t event_mask = 0;
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
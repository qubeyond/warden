#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "common/defs.hpp"

namespace warden::services {

class ScanService;
class FeatureService;
class ModelService;

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
    warden::common::Verdict calculate_verdict(warden::common::FileType type, float max_prob,
                                              size_t suspicious, size_t total) const;

    ScanService& scan_service_;
    FeatureService& feature_service_;
    ModelService& model_service_;
};

}  // namespace warden::services
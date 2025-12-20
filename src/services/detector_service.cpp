#include "services/detector_service.hpp"

#include <algorithm>

namespace warden::services {

DetectorService::DetectorService(ScanService& ss, FeatureService& fs, ModelService& ms)
    : scan_service_(ss), feature_service_(fs), model_service_(ms) {
}

DetectionResult DetectorService::process_file(const std::string& path, float threshold) {
    auto chunks = scan_service_.get_file_chunks(path);
    if (chunks.empty()) {
        return {warden::common::Verdict::UNKNOWN, 0.0f, 0, 0};
    }

    float max_prob = 0.0f;
    size_t suspicious = 0;

    for (const auto& chunk : chunks) {
        auto features = feature_service_.extract_from_buffer(chunk);
        float prob = model_service_.predict(features);

        max_prob = std::max(max_prob, prob);
        if (prob >= threshold) {
            suspicious++;
        }
    }

    warden::common::Verdict final_verdict = warden::common::Verdict::SAFE;
    if (suspicious > 0) {
        final_verdict = (max_prob > 0.8f || suspicious > 2) ? warden::common::Verdict::MALWARE
                                                            : warden::common::Verdict::SUSPICIOUS;
    }

    return {final_verdict, max_prob, suspicious, chunks.size()};
}

}  // namespace warden::services
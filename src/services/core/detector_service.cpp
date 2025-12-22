#include "services/core/detector_service.hpp"

#include <algorithm>

#include "common/defs.hpp"
#include "services/core/feature_service.hpp"
#include "services/core/model_service.hpp"
#include "services/core/scan_service.hpp"

namespace warden::services {

DetectorService::DetectorService(ScanService& ss, FeatureService& fs, ModelService& ms)
    : scan_service_(ss), feature_service_(fs), model_service_(ms) {
}

DetectionResult DetectorService::process_file(const std::string& path, float threshold) {
    const auto chunks = scan_service_.get_file_chunks(path);
    if (chunks.empty()) {
        return {warden::common::Verdict::UNKNOWN, 0.0f, 0, 0, warden::common::FileType::OTHER};
    }

    const auto file_type = feature_service_.identify_file_type(path);
    float max_prob = 0.0f;
    size_t suspicious_count = 0;

    for (const auto& chunk : chunks) {
        const auto feature_set = feature_service_.extract_features(chunk);

        const auto flat_features = feature_set.flatten();

        float prob = model_service_.predict(flat_features);

        float adjusted_prob = prob;
        if (file_type == warden::common::FileType::MEDIA)
            adjusted_prob *= 0.75f;
        else if (file_type == warden::common::FileType::ARCHIVE)
            adjusted_prob *= 0.88f;

        max_prob = std::max(max_prob, adjusted_prob);
        if (adjusted_prob >= threshold) {
            suspicious_count++;
        }
    }

    const warden::common::Verdict final_verdict =
        calculate_verdict(file_type, max_prob, suspicious_count, chunks.size());

    return {final_verdict, max_prob, suspicious_count, chunks.size(), file_type};
}

warden::common::Verdict DetectorService::calculate_verdict(warden::common::FileType type,
                                                           float max_prob, size_t suspicious,
                                                           size_t total) const {
    if (suspicious == 0) return warden::common::Verdict::SAFE;

    float density = static_cast<float>(suspicious) / static_cast<float>(total);

    switch (type) {
        case warden::common::FileType::MEDIA:
            if (max_prob > 0.95f || density > 0.8f) return warden::common::Verdict::MALICIOUS;
            if (max_prob > 0.85f) return warden::common::Verdict::SUSPICIOUS;
            break;

        case warden::common::FileType::ARCHIVE:
            if (max_prob > 0.90f || density > 0.9f) return warden::common::Verdict::MALICIOUS;
            return warden::common::Verdict::SUSPICIOUS;

        case warden::common::FileType::EXECUTABLE:
        case warden::common::FileType::OTHER:
            if (max_prob > 0.85f || suspicious > 3) return warden::common::Verdict::MALICIOUS;
            return warden::common::Verdict::SUSPICIOUS;
    }

    return warden::common::Verdict::SAFE;
}

}  // namespace warden::services
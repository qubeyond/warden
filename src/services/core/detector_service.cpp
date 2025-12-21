#include "services/core/detector_service.hpp"

#include <algorithm>

namespace warden::services {

DetectorService::DetectorService(ScanService& ss, FeatureService& fs, ModelService& ms)
    : scan_service_(ss), feature_service_(fs), model_service_(ms) {
}

DetectionResult DetectorService::process_file(const std::string& path, float threshold) {
    auto chunks = scan_service_.get_file_chunks(path);

    if (chunks.empty()) {
        return {warden::common::Verdict::UNKNOWN, 0.0f, 0, 0, warden::common::FileType::OTHER};
    }

    auto file_type = feature_service_.identify_file_type(path);

    float max_prob = 0.0f;
    size_t suspicious = 0;

    for (const auto& chunk : chunks) {
        auto features = feature_service_.extract_from_buffer(chunk);
        float prob = model_service_.predict(features);

        float adjusted_prob = prob;
        // Применяем штрафы за формат
        if (file_type == warden::common::FileType::MEDIA) adjusted_prob *= 0.75f;
        if (file_type == warden::common::FileType::ARCHIVE) adjusted_prob *= 0.88f;

        max_prob = std::max(max_prob, adjusted_prob);
        if (adjusted_prob >= threshold) suspicious++;
    }

    warden::common::Verdict final_verdict = warden::common::Verdict::SAFE;

    float suspicious_density = static_cast<float>(suspicious) / chunks.size();

    if (suspicious > 0) {
        if (file_type == warden::common::FileType::MEDIA) {
            if (max_prob > 0.95f || suspicious_density > 0.8f)
                final_verdict = warden::common::Verdict::MALWARE;
            else if (max_prob > 0.85f)
                final_verdict = warden::common::Verdict::SUSPICIOUS;
        } else if (file_type == warden::common::FileType::ARCHIVE) {
            if (max_prob > 0.90f || suspicious_density > 0.9f) {
                final_verdict = warden::common::Verdict::MALWARE;
            } else {
                final_verdict = warden::common::Verdict::SUSPICIOUS;
            }
        } else {
            if (max_prob > 0.85f || suspicious > 3)
                final_verdict = warden::common::Verdict::MALWARE;
            else
                final_verdict = warden::common::Verdict::SUSPICIOUS;
        }
    }

    return {final_verdict, max_prob, suspicious, chunks.size(), file_type};
}

}  // namespace warden::services
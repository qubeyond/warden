#include "services/core/feature_service.hpp"

#include <magic.h>

#include <algorithm>
#include <cmath>
#include <numeric>
#include <stdexcept>
#include <string_view>

#include "common/config.hpp"
#include "common/defs.hpp"

namespace warden::services {

std::vector<float> FeatureSet::flatten() const {
    std::vector<float> f;
    f.reserve(262);
    f.insert(f.end(), histogram.begin(), histogram.end());
    f.push_back(entropy);
    f.push_back(mean);
    f.push_back(std_dev);
    f.push_back(autocorrelation);
    f.push_back(chi_square);
    f.push_back(zero_pairs);
    return f;
}

void FeatureService::MagicDeleter::operator()(::magic_set* m) const {
    if (m) magic_close(m);
}

FeatureService::FeatureService(const ConfigManager& config) : config_(config) {
    init_magic();
}

FeatureService::~FeatureService() = default;

void FeatureService::init_magic() {
    auto cookie = magic_open(MAGIC_MIME_TYPE);
    if (!cookie || magic_load(cookie, nullptr) != 0)
        throw std::runtime_error("Failed to initialize libmagic");
    magic_cookie_.reset(cookie);
}

FeatureSet FeatureService::extract_features(const std::vector<uint8_t>& data) const {
    if (config_.model().n_features != 262)
        throw std::runtime_error("FeatureService logic mismatch");

    FeatureSet fs;
    fs.histogram.assign(256, 0.0f);
    if (data.empty()) return fs;

    size_t n = data.size();
    double M2 = 0.0, auto_dot_product = 0.0, sum = 0.0, mean_inc = 0.0;
    uint32_t zp_count = 0;
    std::vector<uint64_t> counts(256, 0);

    for (size_t i = 0; i < n; ++i) {
        uint8_t val = data[i];
        counts[val]++;
        double delta = val - mean_inc;
        mean_inc += delta / (i + 1);
        M2 += delta * (val - mean_inc);
        if (i < n - 1) {
            auto_dot_product += static_cast<double>(val) * data[i + 1];
            if (val == 0 && data[i + 1] == 0) zp_count++;
        }
        sum += val;
    }

    fs.mean = static_cast<float>(mean_inc);
    double variance = (n > 1) ? M2 / n : 0.0;
    fs.std_dev = std::sqrt(static_cast<float>(variance));

    double total_f = static_cast<double>(n);
    for (size_t i = 0; i < 256; ++i) {
        float p = static_cast<float>(counts[i] / total_f);
        fs.histogram[i] = p;
        if (p > 0.0f) fs.entropy -= p * std::log2(p);
    }

    float expected_chi = static_cast<float>(total_f / 256.0);
    for (uint64_t c : counts) {
        float diff = static_cast<float>(c) - expected_chi;
        fs.chi_square += (diff * diff) / expected_chi;
    }

    if (n > 1 && variance > 1e-9) {
        double cov = (auto_dot_product / (total_f - 1)) - (sum * sum / (total_f * (total_f - 1)));
        fs.autocorrelation = static_cast<float>(cov / variance);
    }
    fs.zero_pairs = static_cast<float>(zp_count);
    return fs;
}

warden::common::FileType FeatureService::identify_file_type(const std::string& path) const {
    const char* mime = magic_file(magic_cookie_.get(), path.c_str());
    if (!mime) return warden::common::FileType::OTHER;
    std::string_view s_mime(mime);
    if (s_mime.find("video/") == 0 || s_mime.find("image/") == 0 || s_mime == "application/pdf")
        return warden::common::FileType::MEDIA;
    if (s_mime.find("archive") != std::string_view::npos || s_mime == "application/zip")
        return warden::common::FileType::ARCHIVE;
    if (s_mime.find("executable") != std::string_view::npos ||
        s_mime.find("sharedlib") != std::string_view::npos)
        return warden::common::FileType::EXECUTABLE;
    return warden::common::FileType::OTHER;
}

}  // namespace warden::services
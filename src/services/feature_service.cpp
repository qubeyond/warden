#include "services/feature_service.hpp"

#include <algorithm>
#include <cmath>
#include <numeric>

namespace warden::services {

std::vector<float> FeatureService::extract_from_buffer(const std::vector<uint8_t>& data) {
    if (data.empty()) {
        return std::vector<float>(warden::common::FEATURES_COUNT, 0.0f);
    }

    auto counts = calculate_histogram(data);
    std::vector<float> f(warden::common::FEATURES_COUNT, 0.0f);

    for (size_t i = 0; i < warden::common::HIST_SIZE; ++i) {
        f[i] = static_cast<float>(counts[i]) / data.size();
    }

    float mean = calculate_mean(data);

    f[256] = calculate_entropy(counts, data.size());
    f[257] = mean;
    f[258] = calculate_std(data, mean);
    f[259] = calculate_autocorr(data, mean);
    f[260] = calculate_chi_square(counts, data.size());
    f[261] = 0.0f;
    f[262] = calculate_zero_seq(data);

    return f;
}

std::vector<uint64_t> FeatureService::calculate_histogram(const std::vector<uint8_t>& data) {
    std::vector<uint64_t> counts(warden::common::HIST_SIZE, 0);
    for (uint8_t b : data) counts[b]++;
    return counts;
}

float FeatureService::calculate_entropy(const std::vector<uint64_t>& counts, size_t size) {
    float entropy = 0.0f;
    float total = static_cast<float>(size);
    for (auto c : counts) {
        if (c > 0) {
            float p = static_cast<float>(c) / total;
            entropy -= p * std::log2(p);
        }
    }
    return entropy;
}

float FeatureService::calculate_mean(const std::vector<uint8_t>& data) {
    double sum = std::accumulate(data.begin(), data.end(), 0.0);
    return static_cast<float>(sum / data.size());
}

float FeatureService::calculate_std(const std::vector<uint8_t>& data, float mean) {
    double variance = 0.0;
    for (uint8_t x : data) {
        variance += std::pow(static_cast<float>(x) - mean, 2);
    }
    return std::sqrt(static_cast<float>(variance / data.size()));
}

float FeatureService::calculate_autocorr(const std::vector<uint8_t>& data, float mean) {
    if (data.size() < 2) return 0.0f;
    double num = 0.0, den = 0.0;
    for (size_t i = 0; i < data.size() - 1; ++i) {
        num += (static_cast<float>(data[i]) - mean) * (static_cast<float>(data[i + 1]) - mean);
    }
    for (uint8_t x : data) {
        den += std::pow(static_cast<float>(x) - mean, 2);
    }
    return (den != 0) ? static_cast<float>(num / den) : 0.0f;
}

float FeatureService::calculate_chi_square(const std::vector<uint64_t>& counts, size_t size) {
    float expected = static_cast<float>(size) / warden::common::HIST_SIZE;
    float chi = 0.0f;
    for (auto c : counts) {
        chi += std::pow(static_cast<float>(c) - expected, 2) / expected;
    }
    return chi;
}

float FeatureService::calculate_zero_seq(const std::vector<uint8_t>& data) {
    uint32_t max_z = 0, cur_z = 0;
    for (uint8_t x : data) {
        if (x == 0)
            cur_z++;
        else {
            max_z = std::max(max_z, cur_z);
            cur_z = 0;
        }
    }
    return static_cast<float>(std::max(max_z, cur_z));
}

}  // namespace warden::services
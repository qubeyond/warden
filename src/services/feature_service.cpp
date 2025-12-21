#include "services/feature_service.hpp"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <stdexcept>

namespace warden::services {

FeatureService::FeatureService(const ConfigService& config) : config_(config) {}

std::vector<float> FeatureService::extract_from_buffer(const std::vector<uint8_t>& data) {
    const size_t expected_features = config_.get_features();
    
    if (expected_features != 262) {
        throw std::runtime_error("FeatureService logic mismatch: code expects 262, config says " 
                                 + std::to_string(expected_features));
    }

    if (data.empty()) {
        return std::vector<float>(expected_features, 0.0f);
    }

    auto counts = calculate_histogram(data);
    std::vector<float> f(expected_features, 0.0f);

    float total_size = static_cast<float>(data.size());
    for (size_t i = 0; i < 256; ++i) {
        f[i] = static_cast<float>(counts[i]) / total_size;
    }

    float mean = calculate_mean(data);

    f[256] = calculate_entropy(counts, data.size());
    f[257] = mean;
    f[258] = calculate_std(data, mean);
    f[259] = calculate_autocorr(data, mean);
    f[260] = calculate_chi_square(counts, data.size());
    f[261] = calculate_zero_pairs(data);

    return f;
}

std::vector<uint64_t> FeatureService::calculate_histogram(const std::vector<uint8_t>& data) {
    std::vector<uint64_t> counts(256, 0);
    for (uint8_t b : data) counts[b]++;
    return counts;
}

float FeatureService::calculate_entropy(const std::vector<uint64_t>& counts, size_t size) {
    float ent = 0.0f;
    float total = static_cast<float>(size);
    for (auto c : counts) {
        if (c > 0) {
            float p = static_cast<float>(c) / total;
            ent -= p * std::log2(p);
        }
    }
    return ent;
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
    return (den > 1e-9) ? static_cast<float>(num / den) : 0.0f;
}

float FeatureService::calculate_chi_square(const std::vector<uint64_t>& counts, size_t size) {
    if (size == 0) return 0.0f;
    float expected = static_cast<float>(size) / 256.0f;
    float chi = 0.0f;
    for (auto c : counts) {
        chi += std::pow(static_cast<float>(c) - expected, 2) / expected;
    }
    return chi;
}

float FeatureService::calculate_zero_pairs(const std::vector<uint8_t>& data) {
    if (data.size() < 2) return 0.0f;
    uint32_t pairs = 0;
    for (size_t i = 0; i < data.size() - 1; ++i) {
        if (data[i] == 0 && data[i + 1] == 0) {
            pairs++;
        }
    }
    return static_cast<float>(pairs);
}

}
#include "services/feature_service.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <numeric>

namespace warden::services {

std::vector<float> FeatureService::extract_features(const std::string& file_path) {
    auto data = read_file(file_path);
    if (data.empty()) return {};

    std::vector<uint64_t> counts(256, 0);
    for (uint8_t byte : data) {
        counts[byte]++;
    }

    std::vector<float> features(263, 0.0f);
    float total_size = static_cast<float>(data.size());

    for (int i = 0; i < 256; ++i) {
        features[i] = static_cast<float>(counts[i]) / total_size;
    }

    features[256] = calculate_entropy(counts, data.size());

    double sum_raw = std::accumulate(data.begin(), data.end(), 0.0);
    float mean = static_cast<float>(sum_raw / total_size);
    features[257] = mean;

    double sq_diff_sum = 0.0;
    for (uint8_t x : data) {
        sq_diff_sum += (x - mean) * (x - mean);
    }
    features[258] = static_cast<float>(std::sqrt(sq_diff_sum / total_size));

    if (data.size() > 1) {
        double num = 0.0;
        double den = 0.0;
        for (size_t i = 0; i < data.size() - 1; ++i) {
            num += (data[i] - mean) * (data[i + 1] - mean);
        }
        for (uint8_t x : data) {
            den += (x - mean) * (x - mean);
        }
        features[259] = (den != 0) ? static_cast<float>(num / den) : 0.0f;
    }

    float expected = total_size / 256.0f;
    float chi_stat = 0.0f;
    for (uint64_t count : counts) {
        float diff = static_cast<float>(count) - expected;
        chi_stat += (diff * diff) / expected;
    }
    features[260] = chi_stat;

    features[261] = 0.0f;

    uint32_t max_zeros = 0, current_zeros = 0;
    for (uint8_t x : data) {
        if (x == 0) {
            current_zeros++;
        } else {
            max_zeros = std::max(max_zeros, current_zeros);
            current_zeros = 0;
        }
    }
    features[262] = static_cast<float>(std::max(max_zeros, current_zeros));

    return features;
}

std::vector<uint8_t> FeatureService::read_file(const std::string& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return {};

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(size);
    if (file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        return buffer;
    }
    return {};
}

float FeatureService::calculate_entropy(const std::vector<uint64_t>& counts, size_t total) {
    float entropy = 0.0f;
    float total_f = static_cast<float>(total);

    for (uint64_t count : counts) {
        if (count > 0) {
            float p = static_cast<float>(count) / total_f;
            entropy -= p * std::log2(p);
        }
    }
    return entropy;
}

}  // namespace warden::services
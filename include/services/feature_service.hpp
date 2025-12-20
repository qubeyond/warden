#pragma once
#include <cstdint>
#include <vector>

#include "common/defs.hpp"

namespace warden::services {

class FeatureService {
   public:
    FeatureService() = default;

    std::vector<float> extract_from_buffer(const std::vector<uint8_t>& data);

   private:
    std::vector<uint64_t> calculate_histogram(const std::vector<uint8_t>& data);
    float calculate_entropy(const std::vector<uint64_t>& counts, size_t size);
    float calculate_mean(const std::vector<uint8_t>& data);
    float calculate_std(const std::vector<uint8_t>& data, float mean);
    float calculate_autocorr(const std::vector<uint8_t>& data, float mean);
    float calculate_chi_square(const std::vector<uint64_t>& counts, size_t size);
    float calculate_zero_seq(const std::vector<uint8_t>& data);
};

}  // namespace warden::services
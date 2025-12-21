#pragma once
#include <magic.h>

#include <cstdint>
#include <vector>

#include "services/system/config_service.hpp"

namespace warden::services {

class FeatureService {
   public:
    explicit FeatureService(const ConfigService& config);

    warden::common::FileType identify_file_type(const std::string& path);
    std::vector<float> extract_from_buffer(const std::vector<uint8_t>& data);

   private:
    const ConfigService& config_;

    std::vector<uint64_t> calculate_histogram(const std::vector<uint8_t>& data);
    float calculate_entropy(const std::vector<uint64_t>& counts, size_t size);
    float calculate_mean(const std::vector<uint8_t>& data);
    float calculate_std(const std::vector<uint8_t>& data, float mean);
    float calculate_autocorr(const std::vector<uint8_t>& data, float mean);
    float calculate_chi_square(const std::vector<uint64_t>& counts, size_t size);
    float calculate_zero_pairs(const std::vector<uint8_t>& data);
};

}  // namespace warden::services
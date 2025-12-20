#pragma once
#include <cstdint>
#include <string>
#include <vector>

#include "common/defs.hpp"

namespace warden::services {

class FeatureService {
   public:
    FeatureService() = default;
    std::vector<float> extract_features(const std::string& file_path);

   private:
    std::vector<uint8_t> read_file(const std::string& path);
    float calculate_entropy(const std::vector<uint64_t>& counts, size_t total);
};

}  // namespace warden::services
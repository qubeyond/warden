#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace warden::services {
class ConfigService;
}
namespace warden::common {
enum class FileType : uint8_t;
}

struct magic_set;

namespace warden::services {

struct FeatureSet {
    std::vector<float> histogram;
    float entropy = 0.0f;
    float mean = 0.0f;
    float std_dev = 0.0f;
    float autocorrelation = 0.0f;
    float chi_square = 0.0f;
    float zero_pairs = 0.0f;

    std::vector<float> flatten() const;
};

class FeatureService {
   public:
    explicit FeatureService(const ConfigService& config);
    ~FeatureService();

    warden::common::FileType identify_file_type(const std::string& path) const;
    FeatureSet extract_features(const std::vector<uint8_t>& data) const;

   private:
    const ConfigService& config_;
    struct MagicDeleter {
        void operator()(struct magic_set* m) const;
    };
    std::unique_ptr<struct magic_set, MagicDeleter> magic_cookie_;

    void init_magic();
};

}  // namespace warden::services
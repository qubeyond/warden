#pragma once
#include <cstddef>
#include <cstdint>

namespace warden::common {

enum class Verdict : int8_t { SAFE = 0, MALICIOUS = 1, SUSPICIOUS = 2, UNKNOWN = -1 };

enum class FileType : uint8_t { MEDIA, ARCHIVE, EXECUTABLE, OTHER };

namespace constants {
inline constexpr size_t HIST_SIZE = 256;
inline constexpr size_t CHUNK_SIZE = 4096;
inline constexpr size_t TOTAL_FEATURES = 262;
}  // namespace constants

namespace defaults {
inline constexpr const char* APP_CFG = "configs/app_config.json";
inline constexpr const char* MODEL_CFG = "configs/model_config_v2.json";
inline constexpr const char* PROP_CFG = "configs/properties.json";
}  // namespace defaults

}  // namespace warden::common
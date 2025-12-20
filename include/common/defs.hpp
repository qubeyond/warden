#pragma once

#include <cstdint>
#include <string>

namespace warden::common {

enum class Verdict : int8_t { SAFE = 0, MALWARE = 1, SUSPICIOUS = 2, UNKNOWN = -1 };

inline constexpr size_t HIST_SIZE = 256;
inline constexpr size_t METRICS_SIZE = 7;
inline constexpr size_t FEATURES_COUNT = HIST_SIZE + METRICS_SIZE;

inline constexpr const char *APP_CONFIG_PATH = "configs/app_config.json";
inline constexpr const char *MODEL_CONFIG_PATH = "configs/model_config.json";

}  // namespace warden::common
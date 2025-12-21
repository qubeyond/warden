#pragma once
#include <cstdint>

namespace warden::common {

enum class Verdict : int8_t { 
    SAFE = 0, 
    MALWARE = 1, 
    SUSPICIOUS = 2,
    UNKNOWN = -1
};

enum class FileType : uint8_t {
    MEDIA,      
    ARCHIVE,    
    EXECUTABLE, 
    OTHER       
};

inline constexpr size_t HIST_SIZE = 256;
inline constexpr size_t CHUNK_SIZE = 4096;

inline constexpr const char *DEFAULT_APP_CFG = "configs/app_config.json";
inline constexpr const char *DEFAULT_MODEL_CFG = "configs/model_config_v2.json";

}
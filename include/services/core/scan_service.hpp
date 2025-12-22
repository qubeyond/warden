#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace warden::services {
class ConfigService;
}

namespace warden::services {

class ScanService {
   public:
    explicit ScanService(const ConfigService& config);

    std::vector<std::vector<uint8_t>> get_file_chunks(const std::string& path) const;

   private:
    const ConfigService& config_;
};

}  // namespace warden::services
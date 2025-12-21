#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "common/defs.hpp"
#include "services/config_service.hpp"

namespace warden::services {

class ScanService {
public:
    explicit ScanService(const ConfigService& config);
    std::vector<std::vector<uint8_t>> get_file_chunks(const std::string& path);

private:
    const ConfigService& config_;
    std::vector<uint8_t> read_chunk(std::ifstream& file, size_t offset, size_t file_size);
};

}
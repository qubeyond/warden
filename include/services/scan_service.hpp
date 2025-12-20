#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "common/defs.hpp"

namespace warden::services {

class ScanService {
   public:
    std::vector<std::vector<uint8_t>> get_file_chunks(const std::string& path);

   private:
    std::vector<uint8_t> read_chunk(std::ifstream& file, size_t offset, size_t file_size);
};

}  // namespace warden::services
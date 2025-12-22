#include "services/core/scan_service.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>

#include "common/defs.hpp"
#include "services/system/config_service.hpp"

namespace warden::services {

ScanService::ScanService(const ConfigService& config) : config_(config) {
}

std::vector<std::vector<uint8_t>> ScanService::get_file_chunks(const std::string& path) const {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) return {};

    const auto file_size = static_cast<size_t>(file.tellg());
    if (file_size == 0) return {};

    const size_t chunk_sz = warden::common::constants::CHUNK_SIZE;
    const size_t min_c = config_.get_min_chunks();
    const size_t max_c = config_.get_max_chunks();

    std::vector<size_t> offsets;

    if (file_size <= chunk_sz * min_c) {
        offsets.reserve(file_size / chunk_sz + 1);
        for (size_t offset = 0; offset < file_size; offset += chunk_sz) {
            offsets.push_back(offset);
        }
    } else {
        size_t desired = std::clamp(file_size / (chunk_sz * 20), min_c, max_c);
        offsets.reserve(desired);

        double available_range = static_cast<double>(file_size - chunk_sz);
        double step = available_range / (desired - 1);

        size_t last_offset = 0;
        for (size_t i = 0; i < desired; ++i) {
            size_t current_offset = static_cast<size_t>(std::round(i * step));

            if (i == 0 || current_offset >= last_offset + chunk_sz) {
                offsets.push_back(current_offset);
                last_offset = current_offset;
            }
        }
    }

    std::vector<std::vector<uint8_t>> chunks;
    chunks.reserve(offsets.size());

    for (size_t offset : offsets) {
        file.seekg(offset, std::ios::beg);
        size_t to_read = std::min(chunk_sz, file_size - offset);

        auto& buffer = chunks.emplace_back(chunk_sz, 0);
        file.read(reinterpret_cast<char*>(buffer.data()), to_read);

        if (!file && !file.eof()) {
            chunks.pop_back();
            break;
        }
    }

    return chunks;
}

}  // namespace warden::services
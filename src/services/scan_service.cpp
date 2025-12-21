#include "services/scan_service.hpp"
#include "services/config_service.hpp"
#include "common/defs.hpp"
#include <algorithm>
#include <set>
#include <cmath>
#include <fstream>

namespace warden::services {

ScanService::ScanService(const ConfigService& config) : config_(config) {}

std::vector<std::vector<uint8_t>> ScanService::get_file_chunks(const std::string& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return {};

    size_t file_size = static_cast<size_t>(file.tellg());
    if (file_size == 0) return {};

    std::set<size_t> offsets;
    const size_t chunk_sz = warden::common::CHUNK_SIZE;
    const size_t min_c = config_.get_min_chunks();
    const size_t max_c = config_.get_max_chunks();

    if (file_size <= chunk_sz * min_c) {
        for (size_t offset = 0; offset < file_size; offset += chunk_sz) {
            offsets.insert(offset);
        }
    } else {
        size_t desired = std::clamp(file_size / (chunk_sz * 20), min_c, max_c);
        double step = static_cast<double>(file_size - chunk_sz) / (desired - 1);

        for (size_t i = 0; i < desired; ++i) {
            offsets.insert(static_cast<size_t>(std::round(i * step)));
        }
    }

    std::vector<std::vector<uint8_t>> chunks;
    for (size_t offset : offsets) {
        chunks.push_back(read_chunk(file, offset, file_size));
    }

    return chunks;
}

std::vector<uint8_t> ScanService::read_chunk(std::ifstream& file, size_t offset, size_t file_size) {
    file.seekg(offset, std::ios::beg);
    size_t to_read = std::min(warden::common::CHUNK_SIZE, file_size - offset);

    std::vector<uint8_t> buffer(to_read);
    file.read(reinterpret_cast<char*>(buffer.data()), to_read);

    if (buffer.size() < warden::common::CHUNK_SIZE) {
        buffer.resize(warden::common::CHUNK_SIZE, 0);
    }

    return buffer;
}

}
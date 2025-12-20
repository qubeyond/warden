#include "services/scan_service.hpp"

#include <algorithm>
#include <fstream>
#include <set>

#include "common/defs.hpp"

namespace warden::services {

std::vector<std::vector<uint8_t>> ScanService::get_file_chunks(const std::string& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return {};

    size_t file_size = static_cast<size_t>(file.tellg());
    if (file_size == 0) return {};

    std::set<size_t> offsets;

    offsets.insert(0);

    for (int i = 1; i <= 9; ++i) {
        size_t pos = (file_size * i) / 10;
        if (pos + warden::common::CHUNK_SIZE > file_size) {
            pos = (file_size > warden::common::CHUNK_SIZE)
                      ? (file_size - warden::common::CHUNK_SIZE)
                      : 0;
        }
        offsets.insert(pos);
    }

    if (file_size > warden::common::CHUNK_SIZE) {
        offsets.insert(file_size - warden::common::CHUNK_SIZE);
    }

    std::vector<std::vector<uint8_t>> chunks;
    for (size_t offset : offsets) {
        chunks.push_back(read_chunk(file, offset, file_size));
    }

    return chunks;
}

std::vector<uint8_t> ScanService::read_chunk(std::ifstream& file, size_t offset, size_t file_size) {
    file.seekg(offset, std::ios::beg);
    size_t size_to_read = std::min(warden::common::CHUNK_SIZE, file_size - offset);

    std::vector<uint8_t> buffer(size_to_read);
    file.read(reinterpret_cast<char*>(buffer.data()), size_to_read);

    if (buffer.size() < warden::common::CHUNK_SIZE) {
        buffer.resize(warden::common::CHUNK_SIZE, 0);
    }

    return buffer;
}

}  // namespace warden::services
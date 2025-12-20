#include <iostream>
#include <memory>
#include <vector>

#include "common/defs.hpp"
#include "services/config_service.hpp"

using namespace warden;

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
    try {
        auto config = std::make_shared<services::ConfigService>(common::APP_CONFIG_PATH,
                                                                common::MODEL_CONFIG_PATH);

        std::cout << "[Warden CLI] Core library linked successfully" << std::endl;

        auto dirs = config->get_watch_dirs();
        std::cout << "[Scanner] Monitoring " << dirs.size() << " directories:" << std::endl;
        for (const auto &dir : dirs) {
            std::cout << "  - " << dir << std::endl;
        }

    } catch (const std::exception &e) {
        std::cerr << "Initialization error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
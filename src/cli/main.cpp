#include <exception>
#include <iostream>

#include "common/defs.hpp"
#include "services/cli/cli_service.hpp"
#include "services/core/detector_service.hpp"
#include "services/system/config_service.hpp"

using namespace warden::services;

int main(int argc, char** argv) {
    CliService cli;
    CliOptions options;

    if (!cli.parse(argc, argv, options)) {
        return 0;
    }

    try {
        ConfigService config("configs/app_config.json", "configs/model_config_v2.json",
                             "configs/properties.json");

        ScanService scanner(config);
        FeatureService extractor(config);
        ModelService model(config);

        DetectorService detector(scanner, extractor, model);

        float threshold =
            (options.custom_threshold > 0) ? options.custom_threshold : config.get_threshold();

        auto result = detector.process_file(options.file_path, threshold);
        cli.print_report(options.file_path, result);

    } catch (const std::exception& e) {
        std::cerr << "[!] Fatal Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
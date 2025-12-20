#include "services/config_service.hpp"
#include "services/cli_service.hpp"
#include "services/detector_service.hpp"
#include "common/defs.hpp"

#include <iostream> 
#include <exception>

using namespace warden::services;

int main(int argc, char** argv) {
    CliService cli;
    CliOptions options;

    if (!cli.parse(argc, argv, options)) {
        return 1;
    }

    try {
        ConfigService config(warden::common::APP_CONFIG_PATH, warden::common::MODEL_CONFIG_PATH);
        
        ScanService scanner;
        FeatureService extractor;
        ModelService model(config.get_model_path());
        DetectorService detector(scanner, extractor, model);

        float threshold = (options.custom_threshold > 0) ? options.custom_threshold : config.get_threshold();
        
        auto result = detector.process_file(options.file_path, threshold);
        
        cli.print_report(options.file_path, result);

    } catch (const std::exception& e) {
        std::cerr << "[!] Fatal Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
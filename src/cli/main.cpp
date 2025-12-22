#include <atomic>
#include <csignal>
#include <iostream>
#include <thread>
#include <vector>

// Системные сервисы
#include "services/system/config_service.hpp"
#include "services/system/monitor_service.hpp"

// Кор-сервисы (важно для инициализации объектов)
#include "services/core/detector_service.hpp"
#include "services/core/feature_service.hpp"
#include "services/core/model_service.hpp"
#include "services/core/scan_service.hpp"

// CLI сервис
#include "services/cli/cli_service.hpp"

using namespace warden::services;

// Глобальный флаг для управления циклом из обработчика сигналов
std::atomic<bool> keep_running{true};

// Обработчик сигналов (SIGINT = Ctrl+C)
void signal_handler(int) {
    keep_running = false;
}

// Прослойка (Observer), которая передает результаты из монитора прямо в CLI для печати
class CliObserver : public IReportObserver {
    CliService& cli_;

   public:
    explicit CliObserver(CliService& cli) : cli_(cli) {
    }

    void notify_detection(const std::string& path, const DetectionResult& result) override {
        // Выводим отчет в консоль через CliService
        cli_.print_report(path, result);
    }
};

int main(int argc, char** argv) {
    CliService cli;
    CliOptions options;

    // 1. Парсим аргументы командной строки
    if (!cli.parse(argc, argv, options)) {
        return 0;
    }

    try {
        // 2. Инициализация конфигурации
        ConfigService config("configs/app_config.json", "configs/model_config_v2.json",
                             "configs/properties.json");

        // 3. Инициализация кор-сервисов
        ScanService scanner(config);
        FeatureService extractor(config);
        ModelService model(config);

        // 4. Главный сервис детекции
        DetectorService detector(scanner, extractor, model);

        // Определяем порог срабатывания (из CLI или из конфига)
        float threshold =
            (options.custom_threshold > 0) ? options.custom_threshold : config.get_threshold();

        if (options.mode_monitor) {
            // --- РЕЖИМ МОНИТОРИНГА В РЕАЛЬНОМ ВРЕМЕНИ ---
            CliObserver observer(cli);
            MonitorService monitor(detector, config, observer);

            // Установка обработчика Ctrl+C
            std::signal(SIGINT, signal_handler);

            std::cout << "\033[1;34m[*] Warden AI Real-time Monitor Active\033[0m" << std::endl;
            std::cout << "[*] Watching directory: " << options.monitor_path << std::endl;
            std::cout << "-------------------------------------------------------------------------"
                         "-------"
                      << std::endl;
            std::cout << "VERDICT        | EVENT    | TYPE   | CONF   | CHUNKS  | PATH"
                      << std::endl;
            std::cout << "-------------------------------------------------------------------------"
                         "-------"
                      << std::endl;

            // Запуск мониторинга в отдельном потоке (внутри MonitorService)
            monitor.start({options.monitor_path});

            // Главный поток "засыпает" до получения сигнала прерывания
            while (keep_running) {
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }

            std::cout << "\n[*] Stopping monitor service..." << std::endl;
            monitor.stop();
            std::cout << "[+] Shutdown complete." << std::endl;

        } else {
            // --- РЕЖИМ ОДИНОЧНОГО СКАНА ФАЙЛА ---
            if (options.file_path.empty()) {
                std::cerr << "[!] Error: No file specified for scan. Use 'scan <file>' or 'monitor "
                             "<path>'"
                          << std::endl;
                return 1;
            }

            auto result = detector.process_file(options.file_path, threshold);
            cli.print_report(options.file_path, result);
        }

    } catch (const std::exception& e) {
        std::cerr << "\033[1;31m[!] Fatal Error: " << e.what() << "\033[0m" << std::endl;
        return 1;
    }

    return 0;
}
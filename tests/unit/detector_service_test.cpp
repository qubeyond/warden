#include "services/detector_service.hpp"
#include "services/config_service.hpp"
#include <gtest/gtest.h>
#include <fstream>

using namespace warden::services;
using namespace warden::common;

TEST(DetectorServiceUnitTest, LogicVerdictCalculation) {
    std::ofstream app("unit_app.json");
    app << R"({"database": {"host": "localhost"}, "scanner": {"watch_dirs": []}})";
    app.close();

    std::ofstream model("unit_model.json");
    model << R"({"model_file": "../models/ransomware_model_v1.json", "threshold": 0.5, "n_features": 262, "model_type": "XGB"})";
    model.close();

    // 2. Инициализируем сервисы по новой цепочке зависимостей
    ConfigService cs("unit_app.json", "unit_model.json");
    ScanService ss;
    FeatureService fs(cs);  // Передаем конфиг
    ModelService ms(cs);    // Передаем конфиг
    
    DetectorService ds(ss, fs, ms);

    // Удаляем временные файлы
    std::remove("unit_app.json");
    std::remove("unit_model.json");
}
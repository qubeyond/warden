#include "services/detector_service.hpp"
#include "services/config_service.hpp"
#include <gtest/gtest.h>
#include <fstream>
#include <vector>

using namespace warden::services;
using namespace warden::common;

class DetectorIntegrationTest : public ::testing::Test {
protected:
    // Порядок важен: cs должен быть объявлен ПЕРЕД fs и ms
    std::unique_ptr<ConfigService> cs;
    ScanService ss;
    std::unique_ptr<FeatureService> fs;
    std::unique_ptr<ModelService> ms;

    void SetUp() override {
        // Создаем файлы конфигурации для интеграционных тестов
        std::ofstream app("intg_app.json");
        app << R"({"database": {"host": "localhost"}, "scanner": {"watch_dirs": []}})";
        app.close();

        std::ofstream model("intg_model.json");
        model << R"({"model_file": "../models/ransomware_model_v1.json", "threshold": 0.5, "n_features": 262, "model_type": "XGB"})";
        model.close();

        cs = std::make_unique<ConfigService>("intg_app.json", "intg_model.json");
        fs = std::make_unique<FeatureService>(*cs);
        ms = std::make_unique<ModelService>(*cs);
    }

    void TearDown() override {
        std::remove("intg_app.json");
        std::remove("intg_model.json");
    }

    void create_file(const std::string& p, size_t s, uint8_t b) {
        std::ofstream f(p, std::ios::binary);
        std::vector<uint8_t> d(s, b);
        f.write(reinterpret_cast<const char*>(d.data()), s);
    }
};

TEST_F(DetectorIntegrationTest, BoundarySmallFile) {
    std::string p = "1byte.bin";
    create_file(p, 1, 0x00);
    
    DetectorService ds(ss, *fs, *ms);
    auto res = ds.process_file(p, 0.5f);

    EXPECT_EQ(res.total_chunks, 1);
    std::remove(p.c_str());
}

TEST_F(DetectorIntegrationTest, BoundaryExactlyTenChunks) {
    std::string p = "multi_chunk.bin";
    // Используем CHUNK_SIZE из defs.hpp
    create_file(p, CHUNK_SIZE * 20, 0x00);
    
    DetectorService ds(ss, *fs, *ms);
    auto res = ds.process_file(p, 0.5f);

    EXPECT_GE(res.total_chunks, 11);
    std::remove(p.c_str());
}

TEST_F(DetectorIntegrationTest, NonExistentFile) {
    DetectorService ds(ss, *fs, *ms);
    auto res = ds.process_file("missing_file.exe", 0.5f);
    EXPECT_EQ(res.verdict, Verdict::UNKNOWN);
}
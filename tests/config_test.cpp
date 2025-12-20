#include <gtest/gtest.h>

#include <fstream>

#include "services/config_service.hpp"

using namespace warden::services;

class ConfigServiceTest : public ::testing::Test {
   protected:
    void SetUp() override {
        std::ofstream app("test_app.json");
        app << R"({"database": {"host": "127.0.0.1"}, "scanner": {"watch_dirs": ["/home"]}})";
        app.close();

        std::ofstream model("test_model.json");
        model
            << R"({"model_file": "m.json", "threshold": 0.5, "n_features": 263, "model_type": "XGB"})";
        model.close();
    }

    void TearDown() override {
        std::remove("test_app.json");
        std::remove("test_model.json");
    }
};

TEST_F(ConfigServiceTest, LoadsCorrectValues) {
    ConfigService cs("test_app.json", "test_model.json");

    EXPECT_EQ(cs.get_db_host(), "127.0.0.1");
    EXPECT_FLOAT_EQ(cs.get_threshold(), 0.5f);
    EXPECT_EQ(cs.get_watch_dirs().at(0), "/home");
}

TEST_F(ConfigServiceTest, ThrowsOnMissingFile) {
    EXPECT_THROW(ConfigService("non_existent.json", "test_model.json"), std::runtime_error);
}
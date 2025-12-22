#include "services/system/config_service.hpp"

#include <gtest/gtest.h>

#include <fstream>
#include <memory>

using namespace warden::services;

class ConfigServiceTest : public ::testing::Test {
   protected:
    void SetUp() override {
        std::ofstream app("test_app.json");
        app << R"({"database": {"host": "127.0.0.1", "port": 8123}})";
        app.close();

        std::ofstream model("test_model.json");
        model << R"({"model_file": "m.json", "threshold": 0.5, "n_features": 262})";
        model.close();

        std::ofstream prop("test_prop.json");
        prop << R"({"scanner": {"min_chunks": 5, "max_chunks": 20, "watch_dirs": ["/home"]}})";
        prop.close();
    }

    void TearDown() override {
        std::remove("test_app.json");
        std::remove("test_model.json");
        std::remove("test_prop.json");
    }
};

TEST_F(ConfigServiceTest, LoadsCorrectValues) {
    auto cs = ConfigService::load("test_app.json", "test_model.json", "test_prop.json");
    EXPECT_EQ(cs->app().db_host, "127.0.0.1");
    EXPECT_FLOAT_EQ(cs->model().threshold, 0.5f);
    EXPECT_EQ(cs->scanner().min_chunks, 5);
    EXPECT_EQ(cs->scanner().watch_dirs.at(0), "/home");
}
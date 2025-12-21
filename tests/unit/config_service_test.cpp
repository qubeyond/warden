#include "services/config_service.hpp"
#include <gtest/gtest.h>
#include <fstream>

using namespace warden::services;

class ConfigServiceTest : public ::testing::Test {
protected:
    void SetUp() override {
        std::ofstream app("test_app.json");
        app << R"({"database": {"host": "127.0.0.1"}})";
        app.close();

        std::ofstream model("test_model.json");
        model << R"({"model_file": "m.json", "threshold": 0.5, "n_features": 262, "model_type": "XGB"})";
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
    ConfigService cs("test_app.json", "test_model.json", "test_prop.json");
    EXPECT_EQ(cs.get_db_host(), "127.0.0.1");
    EXPECT_FLOAT_EQ(cs.get_threshold(), 0.5f);
    EXPECT_EQ(cs.get_min_chunks(), 5);
    EXPECT_EQ(cs.get_watch_dirs().at(0), "/home");
}

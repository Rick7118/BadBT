// Unit tests for badbt::parse_service_config (config/service.toml reader).

#include "../include/config_loader.h"

#include <iostream>
#include <fstream>
#include <cstdio>
#include <cmath>
#include <string>

static int g_failures = 0;

#define CHECK(cond, msg)                                         \
    do {                                                         \
        if (!(cond)) {                                           \
            std::cerr << "[FAIL] " << (msg) << " (" #cond ")\n"; \
            ++g_failures;                                        \
        } else {                                                 \
            std::cout << "[ ok ] " << (msg) << "\n";             \
        }                                                        \
    } while (0)

static std::string write_temp(const std::string& contents) {
    std::string p = "test_service_toml_tmp.toml";
    std::ofstream out(p, std::ios::trunc);
    out << contents;
    out.close();
    return p;
}

int main() {
    // 1. A realistic service.toml (mirrors the shipped config/service.toml).
    {
        std::string p = write_temp(
            "# Service configuration for BadBT Detector\n"
            "model_path = \"model/rf_model.onnx\"   # inline comment\n"
            "window_size = 25\n"
            "confidence_threshold = 0.95\n"
            "cooldown_secs = 30\n"
            "device_filter = [\"BTHENUM\", \"BTHLEDevice\"]\n"
            "log_level = \"info\"\n");

        badbt::ServiceConfig c = badbt::parse_service_config(p);
        CHECK(c.found, "file is found and parsed");
        CHECK(c.has_model_path && c.model_path == "model/rf_model.onnx", "model_path parsed, quotes + comment stripped");
        CHECK(c.has_window_size && c.window_size == 25, "window_size parsed as 25");
        CHECK(c.has_threshold && std::fabs(c.threshold - 0.95f) < 1e-6, "confidence_threshold parsed as 0.95");
        CHECK(c.has_cooldown && std::fabs(c.cooldown_seconds - 30.0) < 1e-9, "cooldown_secs parsed as 30");
        CHECK(c.has_filter_bluetooth && c.filter_bluetooth == true, "non-empty device_filter => filter on");
        CHECK(c.has_log_level && c.log_level == "info", "log_level parsed");
        std::remove(p.c_str());
    }

    // 2. Empty device_filter array means "no filter".
    {
        std::string p = write_temp("device_filter = []\n");
        badbt::ServiceConfig c = badbt::parse_service_config(p);
        CHECK(c.has_filter_bluetooth && c.filter_bluetooth == false, "empty device_filter => filter off");
        std::remove(p.c_str());
    }

    // 3. Absent keys keep defaults and report has_*=false (so CLI/defaults are not clobbered).
    {
        std::string p = write_temp("window_size = 10\n");
        badbt::ServiceConfig c = badbt::parse_service_config(p);
        CHECK(c.has_window_size && c.window_size == 10, "present key overrides default");
        CHECK(!c.has_threshold, "absent threshold reports has_threshold=false");
        CHECK(!c.has_model_path, "absent model_path reports has_model_path=false");
        std::remove(p.c_str());
    }

    // 4. Missing file: found=false, defaults intact.
    {
        badbt::ServiceConfig c = badbt::parse_service_config("no_such_config_98765.toml");
        CHECK(!c.found, "missing file reports found=false");
        CHECK(c.window_size == 25 && std::fabs(c.threshold - 0.95f) < 1e-6, "defaults intact when file missing");
    }

    // 5. Garbage / table-header lines are skipped without crashing.
    {
        std::string p = write_temp("[section]\njunk line without equals\nwindow_size = 50\n");
        badbt::ServiceConfig c = badbt::parse_service_config(p);
        CHECK(c.found && c.has_window_size && c.window_size == 50, "malformed lines skipped; valid key still parsed");
        std::remove(p.c_str());
    }

    if (g_failures == 0) {
        std::cout << "\nAll config-loader tests passed.\n";
        return 0;
    }
    std::cerr << "\n" << g_failures << " config-loader test(s) failed.\n";
    return 1;
}

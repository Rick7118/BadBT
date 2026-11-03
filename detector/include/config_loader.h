#pragma once

#include <string>
#include <cstddef>

namespace badbt {

// Parsed contents of config/service.toml.
//
// Only the handful of keys BadBT actually uses are modeled. Each value carries a
// `has_*` flag so callers can implement a clean precedence chain:
//     built-in defaults  ->  service.toml (only keys that were present)  ->  CLI flags
// A key that is absent from the file leaves both the value and its `has_*` flag untouched,
// so it never clobbers a default or a CLI override.
struct ServiceConfig {
    bool found = false;              // true if the file existed and parsed

    std::string model_path = "model/rf_model.onnx";
    std::size_t window_size = 25;
    float       threshold = 0.95f;         // confidence_threshold
    double      cooldown_seconds = 30.0;   // cooldown_secs (retained for compatibility)
    bool        filter_bluetooth = true;   // derived from device_filter (non-empty => true)
    std::string blocklist_path;            // optional override for the persisted blocklist
    std::string log_level = "info";        // parsed; currently informational only

    bool has_model_path = false;
    bool has_window_size = false;
    bool has_threshold = false;
    bool has_cooldown = false;
    bool has_filter_bluetooth = false;
    bool has_blocklist_path = false;
    bool has_log_level = false;
};

// Parse a flat TOML file (string / integer / float / bool / single string-array values,
// `#` line comments, surrounding whitespace). Returns a ServiceConfig with found=false if
// the path does not exist or cannot be opened; malformed individual lines are skipped.
// Intentionally minimal — service.toml is flat, so no general TOML library is vendored.
ServiceConfig parse_service_config(const std::string& path);

// Locate config/service.toml the same way find_model_path() locates the model:
// explicit override first, then next to the executable and up the build tree, then CWD.
// Returns an empty string if no candidate exists on disk.
std::string find_config_path(const std::string& preferred_path = "");

} // namespace badbt

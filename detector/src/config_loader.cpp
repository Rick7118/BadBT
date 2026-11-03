#include "config_loader.h"
#include "path_utils.h"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

namespace badbt {

namespace {

std::string trim(const std::string& s) {
    size_t b = s.find_first_not_of(" \t\r\n");
    if (b == std::string::npos) return "";
    size_t e = s.find_last_not_of(" \t\r\n");
    return s.substr(b, e - b + 1);
}

// Strip an unquoted trailing `# comment`. Leaves `#` inside quotes alone.
std::string strip_comment(const std::string& s) {
    bool in_quotes = false;
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '"') in_quotes = !in_quotes;
        else if (s[i] == '#' && !in_quotes) return s.substr(0, i);
    }
    return s;
}

// Remove one layer of surrounding double quotes, if present.
std::string unquote(const std::string& s) {
    if (s.size() >= 2 && s.front() == '"' && s.back() == '"') {
        return s.substr(1, s.size() - 2);
    }
    return s;
}

// For device_filter = ["BTHENUM", "BTHLEDevice"]: report whether the array holds
// at least one non-empty entry. An explicit empty array ([]) means "no filter".
bool array_has_entries(const std::string& v) {
    size_t lb = v.find('[');
    size_t rb = v.rfind(']');
    if (lb == std::string::npos || rb == std::string::npos || rb < lb) return false;
    std::string inner = trim(v.substr(lb + 1, rb - lb - 1));
    return !inner.empty();
}

} // namespace

ServiceConfig parse_service_config(const std::string& path) {
    ServiceConfig cfg;

    std::ifstream in(path);
    if (!in.is_open()) {
        return cfg; // found stays false
    }
    cfg.found = true;

    std::string line;
    while (std::getline(in, line)) {
        line = trim(strip_comment(line));
        if (line.empty()) continue;

        size_t eq = line.find('=');
        if (eq == std::string::npos) continue; // not a key=value line (e.g. a [table] header)

        std::string key = trim(line.substr(0, eq));
        std::string val = trim(line.substr(eq + 1));
        if (key.empty() || val.empty()) continue;

        if (key == "model_path") {
            cfg.model_path = unquote(val);
            cfg.has_model_path = true;
        } else if (key == "window_size") {
            try {
                long long n = std::stoll(val);
                if (n > 0) { cfg.window_size = static_cast<std::size_t>(n); cfg.has_window_size = true; }
            } catch (...) {}
        } else if (key == "confidence_threshold") {
            try { cfg.threshold = std::stof(val); cfg.has_threshold = true; } catch (...) {}
        } else if (key == "cooldown_secs") {
            try { cfg.cooldown_seconds = std::stod(val); cfg.has_cooldown = true; } catch (...) {}
        } else if (key == "device_filter") {
            cfg.filter_bluetooth = array_has_entries(val);
            cfg.has_filter_bluetooth = true;
        } else if (key == "blocklist_path") {
            cfg.blocklist_path = unquote(val);
            cfg.has_blocklist_path = true;
        } else if (key == "log_level") {
            cfg.log_level = unquote(val);
            cfg.has_log_level = true;
        }
        // Unknown keys are ignored.
    }

    return cfg;
}

std::string find_config_path(const std::string& preferred_path) {
    if (!preferred_path.empty() && file_exists(preferred_path)) {
        return preferred_path;
    }

    std::string exe_dir = get_executable_directory();
    std::vector<std::string> candidates = {
        exe_dir + "\\config\\service.toml",
        exe_dir + "/config/service.toml",
        exe_dir + "\\service.toml",
        exe_dir + "/service.toml",
        exe_dir + "\\..\\..\\config\\service.toml",
        exe_dir + "/../../config/service.toml",
        exe_dir + "\\..\\config\\service.toml",
        exe_dir + "/../config/service.toml",
        "config/service.toml",
        "config\\service.toml"
    };

    for (const auto& cand : candidates) {
        if (file_exists(cand)) {
            return cand;
        }
    }

    return ""; // not found
}

} // namespace badbt

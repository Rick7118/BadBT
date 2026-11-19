#include "blocklist.h"

#include <fstream>

namespace badbt {

namespace {

std::string trim(const std::string& s) {
    size_t b = s.find_first_not_of(" \t\r\n");
    if (b == std::string::npos) return "";
    size_t e = s.find_last_not_of(" \t\r\n");
    return s.substr(b, e - b + 1);
}

} // namespace

void Blocklist::load(const std::string& path) {
    std::lock_guard<std::mutex> lock(mutex_);
    entries_.clear();

    std::ifstream in(path);
    if (!in.is_open()) {
        return; // missing file == empty list
    }
    std::string line;
    while (std::getline(in, line)) {
        std::string entry = trim(line);
        if (!entry.empty()) {
            entries_.insert(entry);
        }
    }
}

bool Blocklist::save(const std::string& path) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::ofstream out(path, std::ios::trunc);
    if (!out.is_open()) {
        return false;
    }
    for (const auto& e : entries_) {
        out << e << "\n";
    }
    return out.good();
}

bool Blocklist::add(const std::string& device_path) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (device_path.empty()) {
        return false;
    }
    auto result = entries_.insert(device_path);
    last_added_ = device_path;
    return result.second;
}

bool Blocklist::contains(const std::string& device_path) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return entries_.find(device_path) != entries_.end();
}

void Blocklist::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    entries_.clear();
    last_added_.clear();
}

std::size_t Blocklist::size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return entries_.size();
}

bool Blocklist::empty() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return entries_.empty();
}

std::vector<std::string> Blocklist::entries() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return std::vector<std::string>(entries_.begin(), entries_.end());
}

std::string Blocklist::last_added() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return last_added_;
}

} // namespace badbt

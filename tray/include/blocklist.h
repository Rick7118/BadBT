#pragma once

#include <string>
#include <vector>
#include <set>
#include <mutex>

namespace badbt {

// Persistent record of device paths that tripped a keystroke-injection alert.
//
// This is the durable "why input is blocked" record. Enforcement itself stays with the
// lock-free is_blocking_active_ atomic in TrayApp, because WH_KEYBOARD_LL cannot act per
// device — so this class is never touched from the hot-path hook. It IS touched from both
// the capture thread (add on alert) and the UI thread (clear on reset), so every operation
// is guarded by a mutex.
//
// Deliberately free of Win32 and JSON dependencies so it can be unit-tested off-Windows.
class Blocklist {
public:
    Blocklist() = default;

    // Load entries from `path`, one UTF-8 device path per line. A missing or unreadable
    // file is treated as an empty list (no throw). Replaces any current contents.
    void load(const std::string& path);

    // Persist current entries to `path`, one per line. Returns false on write failure.
    bool save(const std::string& path) const;

    // Add a device path. Empty strings are ignored. Deduplicated. Returns true if the
    // entry was newly added (false if it was empty or already present).
    bool add(const std::string& device_path);

    bool contains(const std::string& device_path) const;

    // Remove all entries.
    void clear();

    std::size_t size() const;
    bool empty() const;

    // Snapshot of current entries (sorted).
    std::vector<std::string> entries() const;

    // The most recently added device path this session (empty if none added yet).
    std::string last_added() const;

private:
    mutable std::mutex mutex_;
    std::set<std::string> entries_;
    std::string last_added_;
};

} // namespace badbt

// Unit tests for badbt::Blocklist.
// Pure C++ (no Win32 / ONNX) — builds and runs on any platform with clang++/g++ -std=c++17.

#include "../include/blocklist.h"

#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstdint>
#include <string>

static int g_failures = 0;

#define CHECK(cond, msg)                                                   \
    do {                                                                   \
        if (!(cond)) {                                                     \
            std::cerr << "[FAIL] " << (msg) << " (" #cond ")\n";           \
            ++g_failures;                                                  \
        } else {                                                           \
            std::cout << "[ ok ] " << (msg) << "\n";                       \
        }                                                                  \
    } while (0)

static std::string temp_path() {
    // A unique-enough temp path for the test process; no reliance on %APPDATA%.
    std::string p = "test_blocklist_tmp_" + std::to_string(
        static_cast<long long>(reinterpret_cast<std::uintptr_t>(&p))) + ".txt";
    std::remove(p.c_str());
    return p;
}

int main() {
    const std::string DEV_A = "\\\\?\\HID#{00001124-0000-1000-8000-00805f9b34fb}#dev_a";
    const std::string DEV_B = "\\\\?\\HID#{00001812-0000-1000-8000-00805f9b34fb}#dev_b";

    // 1. add / dedup / size
    {
        badbt::Blocklist bl;
        CHECK(bl.empty(), "new blocklist is empty");
        CHECK(bl.add(DEV_A) == true, "first add of a device returns true");
        CHECK(bl.add(DEV_A) == false, "duplicate add returns false");
        CHECK(bl.size() == 1, "duplicate does not grow the list");
        CHECK(bl.add(DEV_B) == true, "second distinct device added");
        CHECK(bl.size() == 2, "two distinct devices held");
        CHECK(bl.contains(DEV_A) && bl.contains(DEV_B), "contains() finds both");
        CHECK(bl.last_added() == DEV_B, "last_added tracks most recent add");
    }

    // 2. empty string is ignored
    {
        badbt::Blocklist bl;
        CHECK(bl.add("") == false, "empty device path is not added");
        CHECK(bl.empty(), "list stays empty after empty add");
    }

    // 3. clear
    {
        badbt::Blocklist bl;
        bl.add(DEV_A);
        bl.add(DEV_B);
        bl.clear();
        CHECK(bl.empty() && bl.size() == 0, "clear empties the list");
        CHECK(bl.last_added().empty(), "clear resets last_added");
    }

    // 4. save -> load round-trip
    {
        std::string path = temp_path();
        badbt::Blocklist a;
        a.add(DEV_A);
        a.add(DEV_B);
        CHECK(a.save(path), "save() reports success");

        badbt::Blocklist b;
        b.load(path);
        CHECK(b.size() == 2, "loaded list has both entries");
        CHECK(b.contains(DEV_A) && b.contains(DEV_B), "loaded list preserves entries");
        std::remove(path.c_str());
    }

    // 5. load of a missing file yields an empty list (no throw)
    {
        badbt::Blocklist bl;
        bl.add(DEV_A); // ensure load() replaces, not appends
        bl.load("definitely_does_not_exist_12345.txt");
        CHECK(bl.empty(), "load of missing file leaves an empty list");
    }

    // 6. save of an empty list persists emptiness (round-trips to empty)
    {
        std::string path = temp_path();
        badbt::Blocklist a;
        a.add(DEV_A);
        a.save(path);
        a.clear();
        CHECK(a.save(path), "save of cleared list succeeds");
        badbt::Blocklist b;
        b.load(path);
        CHECK(b.empty(), "cleared list persists as empty on disk");
        std::remove(path.c_str());
    }

    if (g_failures == 0) {
        std::cout << "\nAll blocklist tests passed.\n";
        return 0;
    }
    std::cerr << "\n" << g_failures << " blocklist test(s) failed.\n";
    return 1;
}

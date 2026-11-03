#pragma once

#include <string>
#include <vector>
#include "device_info.h"

namespace badbt {

// Returns the directory containing the current executable (UTF-8, no trailing slash)
std::string get_executable_directory();

// Searches candidate locations for the ONNX model file and returns the first existing path.
// Searches relative to:
//   1. Explicit override if provided
//   2. Executable dir: <exe_dir>/model/rf_model.onnx
//   3. Executable dir: <exe_dir>/rf_model.onnx
//   4. Build tree relative: <exe_dir>/../../model/rf_model.onnx
//   5. Build tree relative: <exe_dir>/../model/rf_model.onnx
//   6. Current working directory: model/rf_model.onnx
//   7. Current working directory: rf_model.onnx
std::string find_model_path(const std::string& preferred_path = "");

// UTF-8 / UTF-16 conversions
std::wstring utf8_to_wstring(const std::string& str);
// (wstring_to_utf8 is declared in device_info.h)

// Checks if a file exists on disk
bool file_exists(const std::string& path);

// Returns the per-user BadBT application-data directory (e.g. %APPDATA%\BadBT),
// creating it if necessary. Falls back to the current directory if %APPDATA% is
// unavailable. Off-Windows (build-only), returns ".".
std::string get_appdata_dir();

// Resolves the path of the persisted device blocklist file.
//   1. Explicit override if provided (used verbatim; parent dir assumed to exist)
//   2. Otherwise <appdata>/blocklist.txt  (<appdata> is get_appdata_dir())
std::string find_blocklist_path(const std::string& override_path = "");

} // namespace badbt

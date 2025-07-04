#include "revo_uninstaller.h"

void RevoUninstaller::ScanRegistry(const std::wstring& hive_name, const std::wstring& path, const std::wstring& mode) {
    std::map<std::wstring, int> depth_limits = { {L"Quick", 1}, {L"Normal", 3}, {L"Deep", 5}, {L"Custom", 3} };
    int max_depth = depth_limits[mode];
    HKEY hive = hive_name == L"HKEY_CURRENT_USER" ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE;
    std::wstring scan_path = (mode == L"Custom" && !included_keys.empty()) ? included_keys[0] : path;
    
    auto scan_key = [&](HKEY hive, const std::wstring& hive_name, const std::wstring& path, int depth, auto& self) -> void {
        if (depth > max_depth) return;
        bool is_excluded = false;
        for (const auto& ex : excluded_keys) {
            if (path.find(ex) == 0) {
                is_excluded = true;
                break;
            }
        }
        bool is_included = false;
        for (const auto& inc : included_keys) {
            if (path.find(inc) == 0) {
                is_included = true;
                break;
            }
        }
        if (is_excluded && !is_included) {
            scan_results.push_back(L"Skipped excluded key: " + hive_name + L"\\" + path);
            return;
        }

        HKEY key;
        if (RegOpenKeyExW(hive, path.c_str(), 0, KEY_READ | KEY_WRITE, &key) == ERROR_SUCCESS) {
            for (const auto& category : categories) {
                scan_results.push_back(L"Found " + category + L" issue in " + hive_name + L"\\" + path);
            }
            DWORD index = 0;
            WCHAR subkey_name[256];
            DWORD name_len = 256;
            while (RegEnumKeyExW(key, index, subkey_name, &name_len, nullptr, nullptr, nullptr, nullptr) == ERROR_SUCCESS) {
                std::wstring full_path = path.empty() ? subkey_name : path + L"\\" + subkey_name;
                self(hive, hive_name, full_path, depth + 1, self);
                name_len = 256;
                index++;
            }
            RegCloseKey(key);
        } else {
            scan_results.push_back(L"Error accessing " + hive_name + L"\\" + path);
        }
    };

    scan_key(hive, hive_name, scan_path, 0, scan_key);
}

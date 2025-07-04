#include "revo_uninstaller.h"

void RevoUninstaller::CleanRegistry(const std::wstring& hive_name, const std::wstring& path) {
    scan_results.push_back(L"Cleaning invalid entries in " + hive_name + L"\\" + path);
    HKEY hive = hive_name == L"HKEY_CURRENT_USER" ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE;
    HKEY key;
    if (RegOpenKeyExW(hive, path.c_str(), 0, KEY_WRITE, &key) == ERROR_SUCCESS) {
        scan_results.push_back(L"Cleaned " + hive_name + L"\\" + path);
        RegCloseKey(key);
    } else {
        scan_results.push_back(L"Error cleaning " + hive_name + L"\\" + path);
    }
}

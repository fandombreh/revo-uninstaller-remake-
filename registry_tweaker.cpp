#include "revo_uninstaller.h"

void RevoUninstaller::ApplyTweak(const std::wstring& tweak_name) {
    if (tweaks.find(tweak_name) == tweaks.end()) {
        scan_results.push_back(L"Tweak " + tweak_name + L" not found.");
        return;
    }
    auto& tweak = tweaks[tweak_name];
    HKEY hive = tweak.path.find(L"HKEY_CURRENT_USER") == 0 ? HKEY_CURRENT_USER : HKEY_CLASSES_ROOT;
    std::wstring subpath = tweak.path.substr(tweak.path.find(L"\\") + 1);
    HKEY key;
    if (RegCreateKeyExW(hive, subpath.c_str(), 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &key, nullptr) == ERROR_SUCCESS) {
        if (tweak.type == REG_DWORD) {
            DWORD value = std::get<DWORD>(tweak.value);
            RegSetValueExW(key, tweak.value_name.c_str(), 0, REG_DWORD, (BYTE*)&value, sizeof(DWORD));
        } else {
            std::wstring value = std::get<std::wstring>(tweak.value);
            RegSetValueExW(key, tweak.value_name.c_str(), 0, REG_SZ, (BYTE*)value.c_str(), (value.size() + 1) * sizeof(WCHAR));
        }
        scan_results.push_back(L"Applied tweak " + tweak_name + L" at " + tweak.path);
        RegCloseKey(key);
    } else {
        scan_results.push_back(L"Error applying tweak " + tweak_name);
    }
}

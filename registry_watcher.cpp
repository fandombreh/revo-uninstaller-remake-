#include "revo_uninstaller.h"

void RevoUninstaller::RegistryWatcher(const std::wstring& hive_name, const std::wstring& path) {
    auto before = scan_results;
    scan_results.clear();
    ScanRegistry(hive_name, path, L"Quick");
    std::vector<std::wstring> changes;
    for (const auto& result : scan_results) {
        if (std::find(before.begin(), before.end(), result) == before.end()) {
            changes.push_back(result);
        }
    }
    if (!changes.empty()) {
        scan_results.push_back(L"Registry changes detected:");
        scan_results.insert(scan_results.end(), changes.begin(), changes.end());
    } else {
        scan_results.push_back(L"No registry changes detected.");
    }
}

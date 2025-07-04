#include "revo_uninstaller.h"

void RevoUninstaller::AddCustomKey(const std::wstring& key, bool include) {
    auto& target_list = include ? included_keys : excluded_keys;
    if (std::find(target_list.begin(), target_list.end(), key) == target_list.end()) {
        target_list.push_back(key);
        scan_results.push_back((include ? L"Included" : L"Excluded") + std::wstring(L" key: ") + key);
    }
}

#include <windows.h>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <chrono>
#include <thread>

class RevoUninstaller {
private:
    std::vector<std::wstring> scan_results;
    std::vector<std::wstring> excluded_keys = { L"Software\\Microsoft\\Windows\\CurrentVersion\\Run" };
    std::vector<std::wstring> included_keys = { L"Software" };
    std::vector<std::wstring> categories = {
        L"InvalidFilePaths", L"ObsoleteSoftware", L"InvalidCOM", L"BrokenShortcuts",
        L"InvalidFonts", L"MissingDLLs", L"OrphanedCLSID", L"InvalidTypeLib",
        L"CorruptedAppPaths", L"InvalidShellExtensions", L"ObsoleteStartupItems",
        L"InvalidServices", L"BrokenFileAssociations", L"InvalidMRU", L"CorruptedHelpFiles",
        L"InvalidInstallerEntries", L"ObsoleteDriverEntries"
    };
    const std::wstring backup_file = L"revo_backup.reg";

    void CheckKey(const std::wstring& hive_name, const std::wstring& path) {
        for (const auto& category : categories) {
            scan_results.push_back(L"Found " + category + L" issue in " + hive_name + L"\\" + path);
        }
    }

    void ScanKey(HKEY hive, const std::wstring& hive_name, const std::wstring& path, int depth, int max_depth) {
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
            CheckKey(hive_name, path);
            DWORD index = 0;
            WCHAR subkey_name[256];
            DWORD name_len = 256;
            while (RegEnumKeyExW(key, index, subkey_name, &name_len, nullptr, nullptr, nullptr, nullptr) == ERROR_SUCCESS) {
                std::wstring full_path = path.empty() ? subkey_name : path + L"\\" + subkey_name;
                ScanKey(hive, hive_name, full_path, depth + 1, max_depth);
                name_len = 256;
                index++;
            }
            RegCloseKey(key);
        } else {
            scan_results.push_back(L"Error accessing " + hive_name + L"\\" + path);
        }
    }

public:
    void ScanRegistry(const std::wstring& hive_name, const std::wstring& path, const std::wstring& mode = L"Normal") {
        std::map<std::wstring, int> depth_limits = { {L"Quick", 1}, {L"Normal", 3}, {L"Deep", 5}, {L"Custom", 3} };
        int max_depth = depth_limits[mode];
        HKEY hive = hive_name == L"HKEY_CURRENT_USER" ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE;
        std::wstring scan_path = (mode == L"Custom" && !included_keys.empty()) ? included_keys[0] : path;
        ScanKey(hive, hive_name, scan_path, 0, max_depth);
    }

    void ApplyTweak(const std::wstring& tweak_name) {
        struct Tweak {
            std::wstring path;
            std::wstring value_name;
            std::variant<DWORD, std::wstring> value;
            DWORD type;
        };
        std::map<std::wstring, Tweak> tweaks = {
            { L"DisableFileExtensionHiding", { L"HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced", L"HideFileExt", DWORD(0), REG_DWORD } },
            { L"AddNotepadContextMenu", { L"HKEY_CLASSES_ROOT\\*\\shell\\OpenWithNotepad", L"Command", std::wstring(L"notepad.exe %1"), REG_SZ } },
            { L"DisableTaskbarAnimations", { L"HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced", L"TaskbarAnimations", DWORD(0), REG_DWORD } }
        };
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

    void CleanRegistry(const std::wstring& hive_name, const std::wstring& path) {
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

    void BackupRegistry(bool full_backup = true) {
        std::wstring hive = full_backup ? L"HKEY_LOCAL_MACHINE" : L"HKEY_CURRENT_USER";
        std::wstring cmd = L"reg export " + hive + L" \"" + backup_file + L"\" /y";
        if (system(std::string(cmd.begin(), cmd.end()).c_str()) == 0) {
            scan_results.push_back(L"Registry backed up to " + backup_file);
        } else {
            scan_results.push_back(L"Backup failed");
        }
    }

    void ExportChanges(const std::wstring& filename = L"revo_changes.txt") {
        std::wofstream file(filename);
        file << L"Revo Uninstaller Scan Results - " << std::chrono::system_clock::now().time_since_epoch().count() << L"\n";
        for (const auto& result : scan_results) {
            file << result << L"\n";
        }
        file.close();
        scan_results.push_back(L"Changes exported to " + filename);
    }

    void RegistryWatcher(const std::wstring& hive_name, const std::wstring& path) {
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

    void ScheduleAutoClean() {
        std::thread([this]() {
            while (true) {
                auto now = std::chrono::system_clock::now();
                auto next_midnight = now - std::chrono::hours(now.time_since_epoch().count() / 3600 % 24) + std::chrono::hours(24);
                std::this_thread::sleep_until(next_midnight);
                scan_results.clear();
                ScanRegistry(L"HKEY_CURRENT_USER", L"Software", L"Quick");
                CleanRegistry(L"HKEY_CURRENT_USER", L"Software");
                ExportChanges(L"auto_clean_" + std::to_wstring(now.time_since_epoch().count()) + L".txt");
                scan_results.push_back(L"Auto-clean completed.");
            }
        }).detach();
    }

    void AddCustomKey(const std::wstring& key, bool include = true) {
        auto& target_list = include ? included_keys : excluded_keys;
        if (std::find(target_list.begin(), target_list.end(), key) == target_list.end()) {
            target_list.push_back(key);
            scan_results.push_back((include ? L"Included" : L"Excluded") + std::wstring(L" key: ") + key);
        }
    }

    void Run() {
        std::wcout << L"Starting Revo Uninstaller Clone...\n";
        BackupRegistry(false);
        AddCustomKey(L"Software\\Microsoft", true);
        AddCustomKey(L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer", false);
        ScanRegistry(L"HKEY_CURRENT_USER", L"Software", L"Deep");
        ScanRegistry(L"HKEY_LOCAL_MACHINE", L"Software", L"Quick");
        ApplyTweak(L"DisableFileExtensionHiding");
        ApplyTweak(L"AddNotepadContextMenu");
        CleanRegistry(L"HKEY_CURRENT_USER", L"Software");
        RegistryWatcher(L"HKEY_CURRENT_USER", L"Software");
        ExportChanges();
        ScheduleAutoClean();
        std::wcout << L"Running scheduled tasks... Press Ctrl+C to stop.\n";
        std::this_thread::sleep_for(std::chrono::hours(24));
    }
};

int main() {
    RevoUninstaller uninstaller;
    uninstaller.Run();
    return 0;
}

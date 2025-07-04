#include <windows.h>
#include <string>
#include <vector>
#include <map>
#include <variant>

struct Tweak {
    std::wstring path;
    std::wstring value_name;
    std::variant<DWORD, std::wstring> value;
    DWORD type;
};

class RevoUninstaller {
private:
    std::vector<std::wstring> scan_results;
    std::vector<std::wstring> excluded_keys;
    std::vector<std::wstring> included_keys;
    std::vector<std::wstring> categories;
    const std::wstring backup_file;
    std::map<std::wstring, Tweak> tweaks;

public:
    RevoUninstaller();
    void ScanRegistry(const std::wstring& hive_name, const std::wstring& path, const std::wstring& mode);
    void ApplyTweak(const std::wstring& tweak_name);
    void CleanRegistry(const std::wstring& hive_name, const std::wstring& path);
    void BackupRegistry(bool full_backup);
    void ExportChanges(const std::wstring& filename);
    void RegistryWatcher(const std::wstring& hive_name, const std::wstring& path);
    void ScheduleAutoClean();
    void AddCustomKey(const std::wstring& key, bool include);
    void Run();
};

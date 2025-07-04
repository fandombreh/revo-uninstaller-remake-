#include "revo_uninstaller.h"

void RevoUninstaller::BackupRegistry(bool full_backup) {
    std::wstring hive = full_backup ? L"HKEY_LOCAL_MACHINE" : L"HKEY_CURRENT_USER";
    std::wstring cmd = L"reg export " + hive + L" \"" + backup_file + L"\" /y";
    if (system(std::string(cmd.begin(), cmd.end()).c_str()) == 0) {
        scan_results.push_back(L"Registry backed up to " + backup_file);
    } else {
        scan_results.push_back(L"Backup failed");
    }
}

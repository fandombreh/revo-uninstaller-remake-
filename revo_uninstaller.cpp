#include "revo_uninstaller.h"
#include <iostream>

RevoUninstaller::RevoUninstaller()
    : backup_file(L"revo_backup.reg"),
      excluded_keys({ L"Software\\Microsoft\\Windows\\CurrentVersion\\Run" }),
      included_keys({ L"Software" }),
      categories({
          L"InvalidFilePaths", L"ObsoleteSoftware", L"InvalidCOM", L"BrokenShortcuts",
          L"InvalidFonts", L"MissingDLLs", L"OrphanedCLSID", L"InvalidTypeLib",
          L"CorruptedAppPaths", L"InvalidShellExtensions", L"ObsoleteStartupItems",
          L"InvalidServices", L"BrokenFileAssociations", L"InvalidMRU", L"CorruptedHelpFiles",
          L"InvalidInstallerEntries", L"ObsoleteDriverEntries"
      }),
      tweaks({
          { L"DisableFileExtensionHiding", { L"HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced", L"HideFileExt", DWORD(0), REG_DWORD } },
          { L"AddNotepadContextMenu", { L"HKEY_CLASSES_ROOT\\*\\shell\\OpenWithNotepad", L"Command", std::wstring(L"notepad.exe %1"), REG_SZ } },
          { L"DisableTaskbarAnimations", { L"HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced", L"TaskbarAnimations", DWORD(0), REG_DWORD } }
      }) {}

void RevoUninstaller::Run() {
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

int main() {
    RevoUninstaller uninstaller;
    uninstaller.Run();
    return 0;
}

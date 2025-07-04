#include "revo_uninstaller.h"
#include <thread>
#include <chrono>

void RevoUninstaller::ScheduleAutoClean() {
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

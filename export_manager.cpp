#include "revo_uninstaller.h"
#include <chrono>
#include <fstream>

void RevoUninstaller::ExportChanges(const std::wstring& filename) {
    std::wofstream file(filename);
    file << L"Revo Uninstaller Scan Results - " << std::chrono::system_clock::now().time_since_epoch().count() << L"\n";
    for (const auto& result : scan_results) {
        file << result << L"\n";
    }
    file.close();
    scan_results.push_back(L"Changes exported to " + filename);
}

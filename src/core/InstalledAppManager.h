#pragma once

#include <vector>
#include <string>
#include <optional>
#include <functional>
#include "Types.h"

namespace pulse {

class InstalledAppManager {
public:
    enum class SortField {
        NameAsc,
        NameDesc,
        SizeDesc,
        SizeAsc,
        DateDesc,
        DateAsc,
        PublisherAsc
    };

    InstalledAppManager() = default;
    virtual ~InstalledAppManager() = default;

    // Scans Windows Registry (HKLM 64-bit, HKLM 32-bit, HKCU) for registered applications
    [[nodiscard]] virtual std::vector<InstalledAppInfo> scanInstalledApplications(
        const std::vector<ProcessInfo>& runningProcesses = {});

    // Safe execution of registered Windows uninstaller
    [[nodiscard]] virtual UninstallResult uninstallApplication(
        const InstalledAppInfo& app,
        bool waitForCompletion = true,
        uint32_t timeoutMs = 180000);

    // Filtering and sorting helpers
    [[nodiscard]] static std::vector<InstalledAppInfo> filterByCategory(
        const std::vector<InstalledAppInfo>& apps,
        AppCategory category);

    [[nodiscard]] static std::vector<InstalledAppInfo> filterByName(
        const std::vector<InstalledAppInfo>& apps,
        const std::string& query);

    static void sortApplications(
        std::vector<InstalledAppInfo>& apps,
        SortField field);

    // Formats byte count to clean string (e.g. "1.24 GB", "450 MB")
    [[nodiscard]] static std::string formatBytes(uint64_t bytes);

private:
    void scanRegistryHive(void* hKeyRoot,
                          const wchar_t* subKeyPath,
                          const std::string& rootName,
                          unsigned long accessFlags,
                          const std::vector<ProcessInfo>& runningProcesses,
                          std::vector<InstalledAppInfo>& results);

    void evaluateUsageEvidence(InstalledAppInfo& app,
                               const std::vector<ProcessInfo>& runningProcesses);
};

} // namespace pulse


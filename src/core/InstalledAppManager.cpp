#include "InstalledAppManager.h"
#include "DiagnosticLog.h"

#include <windows.h>
#include <shellapi.h>
#include <filesystem>
#include <chrono>
#include <algorithm>
#include <sstream>
#include <cctype>
#include <set>
#include <iomanip>

namespace pulse {

namespace {

// Helper: UTF-16 wstring to UTF-8 string
std::string wideToUtf8(const std::wstring& wstr) {
    if (wstr.empty()) return {};
    int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), static_cast<int>(wstr.size()),
                                         nullptr, 0, nullptr, nullptr);
    if (sizeNeeded <= 0) return {};
    std::string result(sizeNeeded, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.data(), static_cast<int>(wstr.size()),
                        result.data(), sizeNeeded, nullptr, nullptr);
    return result;
}

// Helper: UTF-8 string to UTF-16 wstring
std::wstring utf8ToWide(const std::string& str) {
    if (str.empty()) return {};
    int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.size()),
                                         nullptr, 0);
    if (sizeNeeded <= 0) return {};
    std::wstring result(sizeNeeded, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.size()),
                        result.data(), sizeNeeded);
    return result;
}

// Helper: Case-insensitive lowercase
std::string toLower(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return str;
}

// Helper: Trim whitespace and quotes
std::string trim(std::string str) {
    while (!str.empty() && (std::isspace(static_cast<unsigned char>(str.front())) || str.front() == '"')) {
        str.erase(str.begin());
    }
    while (!str.empty() && (std::isspace(static_cast<unsigned char>(str.back())) || str.back() == '"')) {
        str.pop_back();
    }
    return str;
}

// Helper: Read string value from Registry
std::wstring queryRegString(HKEY hKey, const wchar_t* valName) {
    wchar_t buffer[1024] = {0};
    DWORD bufSize = sizeof(buffer);
    DWORD type = 0;
    LSTATUS status = RegQueryValueExW(hKey, valName, nullptr, &type,
                                      reinterpret_cast<LPBYTE>(buffer), &bufSize);
    if (status == ERROR_SUCCESS && (type == REG_SZ || type == REG_EXPAND_SZ)) {
        if (type == REG_EXPAND_SZ) {
            wchar_t expanded[2048] = {0};
            ExpandEnvironmentStringsW(buffer, expanded, 2048);
            return std::wstring(expanded);
        }
        return std::wstring(buffer);
    }
    return {};
}

// Helper: Read DWORD value from Registry
DWORD queryRegDword(HKEY hKey, const wchar_t* valName, DWORD defaultVal = 0) {
    DWORD val = 0;
    DWORD bufSize = sizeof(val);
    DWORD type = 0;
    LSTATUS status = RegQueryValueExW(hKey, valName, nullptr, &type,
                                      reinterpret_cast<LPBYTE>(&val), &bufSize);
    if (status == ERROR_SUCCESS && type == REG_DWORD) {
        return val;
    }
    return defaultVal;
}

// Helper: Parse raw Windows install date (YYYYMMDD) to readable format (YYYY-MM-DD)
std::string formatRawDate(const std::string& raw) {
    if (raw.size() == 8 && std::all_of(raw.begin(), raw.end(), ::isdigit)) {
        return raw.substr(0, 4) + "-" + raw.substr(4, 2) + "-" + raw.substr(6, 2);
    }
    return raw;
}

} // anonymous namespace

std::vector<InstalledAppInfo> InstalledAppManager::scanInstalledApplications(
    const std::vector<ProcessInfo>& runningProcesses) {

    logDiagnostic("[InstalledAppManager] Starting scan of registered Windows applications...");
    std::vector<InstalledAppInfo> results;
    std::set<std::string> seenIds;

    const wchar_t* uninstallSubKey = L"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall";

    // 1. HKLM 64-bit
    scanRegistryHive(HKEY_LOCAL_MACHINE, uninstallSubKey, "HKLM",
                     KEY_READ | KEY_WOW64_64KEY, runningProcesses, results);

    // 2. HKLM 32-bit (Wow6432Node)
    scanRegistryHive(HKEY_LOCAL_MACHINE, uninstallSubKey, "HKLM_32",
                     KEY_READ | KEY_WOW64_32KEY, runningProcesses, results);

    // 3. HKCU (Current User installations)
    scanRegistryHive(HKEY_CURRENT_USER, uninstallSubKey, "HKCU",
                     KEY_READ, runningProcesses, results);

    // Deduplicate by Name + Version
    std::vector<InstalledAppInfo> deduplicated;
    for (auto& app : results) {
        std::string dedupeKey = toLower(app.name) + "|" + toLower(app.version);
        if (seenIds.find(dedupeKey) == seenIds.end()) {
            seenIds.insert(dedupeKey);
            deduplicated.push_back(std::move(app));
        }
    }

    logDiagnostic("[InstalledAppManager] Scan complete. Found " +
                  std::to_string(deduplicated.size()) + " unique registered applications.");
    return deduplicated;
}

void InstalledAppManager::scanRegistryHive(
    void* hKeyRoot,
    const wchar_t* subKeyPath,
    const std::string& rootName,
    unsigned long accessFlags,
    const std::vector<ProcessInfo>& runningProcesses,
    std::vector<InstalledAppInfo>& results) {

    HKEY hRoot = reinterpret_cast<HKEY>(hKeyRoot);
    HKEY hUninstallKey = nullptr;
    LSTATUS status = RegOpenKeyExW(hRoot, subKeyPath, 0, accessFlags, &hUninstallKey);
    if (status != ERROR_SUCCESS || !hUninstallKey) {
        return;
    }

    DWORD subKeysCount = 0;
    DWORD maxSubKeyLen = 0;
    RegQueryInfoKeyW(hUninstallKey, nullptr, nullptr, nullptr, &subKeysCount,
                     &maxSubKeyLen, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);

    std::vector<wchar_t> keyNameBuf(maxSubKeyLen + 2, 0);

    for (DWORD i = 0; i < subKeysCount; ++i) {
        DWORD keyNameLen = static_cast<DWORD>(keyNameBuf.size());
        status = RegEnumKeyExW(hUninstallKey, i, keyNameBuf.data(), &keyNameLen,
                               nullptr, nullptr, nullptr, nullptr);
        if (status != ERROR_SUCCESS) continue;

        HKEY hAppKey = nullptr;
        status = RegOpenKeyExW(hUninstallKey, keyNameBuf.data(), 0, accessFlags, &hAppKey);
        if (status != ERROR_SUCCESS || !hAppKey) continue;

        std::wstring wsName = queryRegString(hAppKey, L"DisplayName");
        if (wsName.empty()) {
            RegCloseKey(hAppKey);
            continue;
        }

        std::string name = wideToUtf8(wsName);

        // Filter system components and updates
        DWORD isSysComponent = queryRegDword(hAppKey, L"SystemComponent", 0);
        DWORD noRemove = queryRegDword(hAppKey, L"NoRemove", 0);
        std::wstring wsParentKey = queryRegString(hAppKey, L"ParentKeyName");
        std::wstring wsReleaseType = queryRegString(hAppKey, L"ReleaseType");

        bool isUpdate = !wsParentKey.empty() ||
                        toLower(wideToUtf8(wsReleaseType)).find("update") != std::string::npos ||
                        toLower(wideToUtf8(wsReleaseType)).find("hotfix") != std::string::npos ||
                        (name.rfind("KB", 0) == 0 && name.size() >= 8 && std::isdigit(static_cast<unsigned char>(name[2])));

        // Skip Windows updates and hidden components from the application list
        if (isUpdate || isSysComponent == 1) {
            RegCloseKey(hAppKey);
            continue;
        }

        InstalledAppInfo app;
        app.id = wideToUtf8(keyNameBuf.data());
        app.name = name;
        app.displayName = name;
        app.publisher = wideToUtf8(queryRegString(hAppKey, L"Publisher"));
        if (app.publisher.empty()) app.publisher = "Unknown Publisher";

        app.version = wideToUtf8(queryRegString(hAppKey, L"DisplayVersion"));
        app.displayVersion = app.version;
        app.installDateRaw = wideToUtf8(queryRegString(hAppKey, L"InstallDate"));
        app.installDate = formatRawDate(app.installDateRaw);

        DWORD sizeKb = queryRegDword(hAppKey, L"EstimatedSize", 0);
        app.estimatedSizeBytes = static_cast<uint64_t>(sizeKb) * 1024ULL;
        app.estimatedSize = app.estimatedSizeBytes;

        app.installLocation = wideToUtf8(queryRegString(hAppKey, L"InstallLocation"));
        app.uninstallString = wideToUtf8(queryRegString(hAppKey, L"UninstallString"));
        app.quietUninstallString = wideToUtf8(queryRegString(hAppKey, L"QuietUninstallString"));
        app.registryRoot = rootName;
        app.isSystemComponent = (isSysComponent == 1);
        app.isWindowsUpdate = isUpdate;
        app.canUninstall = (!app.uninstallString.empty() && noRemove == 0 && !app.isSystemComponent);

        // Inspect file system timestamp & process presence for evidence-based categorization
        evaluateUsageEvidence(app, runningProcesses);

        results.push_back(std::move(app));
        RegCloseKey(hAppKey);
    }

    RegCloseKey(hUninstallKey);
}

void InstalledAppManager::evaluateUsageEvidence(
    InstalledAppInfo& app,
    const std::vector<ProcessInfo>& runningProcesses) {

    // 1. Process activity check
    std::string lowerName = toLower(app.name);
    std::string lowerLocation = toLower(app.installLocation);
    app.isCurrentlyRunning = false;

    for (const auto& proc : runningProcesses) {
        std::string procExeLower = toLower(proc.executablePath);
        std::string procNameLower = toLower(proc.name);

        if (!lowerLocation.empty() && procExeLower.find(lowerLocation) != std::string::npos) {
            app.isCurrentlyRunning = true;
            break;
        }
        if (lowerName.find(procNameLower.substr(0, procNameLower.find_last_of('.'))) != std::string::npos) {
            app.isCurrentlyRunning = true;
            break;
        }
    }

    // 2. File system timestamp verification
    int daysSinceLastWrite = -1;
    if (!app.installLocation.empty()) {
        std::error_code ec;
        std::filesystem::path p(app.installLocation);
        if (std::filesystem::exists(p, ec)) {
            auto ftime = std::filesystem::last_write_time(p, ec);
            if (!ec) {
                auto sctime = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                    ftime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());
                app.lastWriteTime = sctime;
                auto now = std::chrono::system_clock::now();
                daysSinceLastWrite = static_cast<int>(
                    std::chrono::duration_cast<std::chrono::hours>(now - sctime).count() / 24);
            }
        }
    }

    // 3. Category classification with verifiable evidence
    if (app.estimatedSizeBytes >= 500 * 1024 * 1024) { // 500 MB+
        app.category = AppCategory::Large;
    } else if (app.publisher == "Unknown Publisher") {
        app.category = AppCategory::UnknownPublisher;
    } else {
        app.category = AppCategory::All;
    }

    // Check recently installed (e.g. within 60 days)
    if (!app.installDateRaw.empty() && app.installDateRaw.size() == 8) {
        try {
            int yr = std::stoi(app.installDateRaw.substr(0, 4));
            int mo = std::stoi(app.installDateRaw.substr(4, 2));
            int dy = std::stoi(app.installDateRaw.substr(6, 2));
            std::tm tmVal = {};
            tmVal.tm_year = yr - 1900;
            tmVal.tm_mon = mo - 1;
            tmVal.tm_mday = dy;
            auto installTp = std::chrono::system_clock::from_time_t(std::mktime(&tmVal));
            auto now = std::chrono::system_clock::now();
            auto daysSinceInstall = std::chrono::duration_cast<std::chrono::hours>(now - installTp).count() / 24;
            if (daysSinceInstall >= 0 && daysSinceInstall <= 60) {
                app.category = AppCategory::RecentlyInstalled;
            }
        } catch (...) {}
    }

    // Potentially unused classification: requires BOTH zero active processes AND aged directory/install date
    if (!app.isCurrentlyRunning && (daysSinceLastWrite > 90 || app.category == AppCategory::Large)) {
        if (daysSinceLastWrite > 90) {
            app.category = AppCategory::PotentiallyUnused;
            app.usageEvidence = "Usage Evidence: Zero active processes running; installation files untouched for " +
                                std::to_string(daysSinceLastWrite) + " days.";
        }
    }

    if (app.usageEvidence.empty()) {
        if (app.isCurrentlyRunning) {
            app.usageEvidence = "Usage Evidence: Currently active in background/foreground processes.";
        } else if (!app.installDate.empty()) {
            app.usageEvidence = "Usage Evidence: Registered on " + app.installDate + "; inactive in current session.";
        } else {
            app.usageEvidence = "Usage Evidence: No running process detected in current session.";
        }
    }
}

UninstallResult InstalledAppManager::uninstallApplication(
    const InstalledAppInfo& app,
    bool waitForCompletion,
    uint32_t timeoutMs) {

    logDiagnostic("[InstalledAppManager] Initiating uninstall request for: " + app.name);

    if (app.isSystemComponent) {
        logDiagnostic("[InstalledAppManager] Denied: Application is a protected system component.");
        return {false, 0, "Action aborted: This application is a protected Windows system component."};
    }

    if (app.uninstallString.empty()) {
        logDiagnostic("[InstalledAppManager] Denied: No registered uninstall string found.");
        return {false, 0, "No registered Windows uninstaller found for this application."};
    }

    std::string uninstCmd = app.uninstallString;
    std::string exePath;
    std::string args;

    // Parse command line into executable path and arguments
    if (uninstCmd.front() == '"') {
        size_t closingQuote = uninstCmd.find('"', 1);
        if (closingQuote != std::string::npos) {
            exePath = uninstCmd.substr(1, closingQuote - 1);
            if (closingQuote + 1 < uninstCmd.size()) {
                args = trim(uninstCmd.substr(closingQuote + 1));
            }
        } else {
            exePath = uninstCmd.substr(1);
        }
    } else {
        // Special case: MsiExec.exe /I{...} or /X{...}
        std::string lowerCmd = toLower(uninstCmd);
        if (lowerCmd.find("msiexec") != std::string::npos) {
            exePath = "msiexec.exe";
            size_t paramStart = lowerCmd.find("msiexec");
            paramStart += 7; // length of msiexec
            if (paramStart < uninstCmd.size()) {
                args = trim(uninstCmd.substr(paramStart));
                // Convert /I to /X for uninstallation if needed
                size_t slashI = args.find("/I");
                if (slashI == std::string::npos) slashI = args.find("/i");
                if (slashI != std::string::npos) {
                    args.replace(slashI, 2, "/X");
                }
            }
        } else {
            size_t firstSpace = uninstCmd.find(' ');
            if (firstSpace != std::string::npos) {
                exePath = uninstCmd.substr(0, firstSpace);
                args = trim(uninstCmd.substr(firstSpace + 1));
            } else {
                exePath = uninstCmd;
            }
        }
    }

    std::wstring wExe = utf8ToWide(exePath);
    std::wstring wArgs = utf8ToWide(args);

    logDiagnostic("[InstalledAppManager] Executing uninstaller: " + exePath + " with parameters: " + args);

    SHELLEXECUTEINFOW sei = { sizeof(sei) };
    sei.fMask = SEE_MASK_NOCLOSEPROCESS;
    sei.hwnd = nullptr;
    sei.lpVerb = L"open";
    sei.lpFile = wExe.c_str();
    sei.lpParameters = wArgs.empty() ? nullptr : wArgs.c_str();
    sei.nShow = SW_SHOWNORMAL;

    if (!ShellExecuteExW(&sei)) {
        DWORD err = GetLastError();
        // If elevation required, retry with "runas"
        if (err == ERROR_ELEVATION_REQUIRED) {
            logDiagnostic("[InstalledAppManager] Elevation required; prompting UAC via runas...");
            sei.lpVerb = L"runas";
            if (!ShellExecuteExW(&sei)) {
                err = GetLastError();
                if (err == ERROR_CANCELLED) {
                    return {false, err, "Uninstallation cancelled by user (administrator elevation declined)."};
                }
                return {false, err, "Failed to launch uninstaller with administrator elevation. Error code: " + std::to_string(err)};
            }
        } else if (err == ERROR_CANCELLED) {
            return {false, err, "Uninstallation cancelled by user."};
        } else {
            return {false, err, "Failed to execute registered uninstaller. Error code: " + std::to_string(err)};
        }
    }

    if (!sei.hProcess) {
        return {true, 0, "Uninstaller launched successfully."};
    }

    if (!waitForCompletion) {
        CloseHandle(sei.hProcess);
        return {true, 0, "Uninstaller process launched successfully."};
    }

    // Wait for the uninstaller process with timeout
    DWORD waitRes = WaitForSingleObject(sei.hProcess, timeoutMs);
    if (waitRes == WAIT_OBJECT_0) {
        DWORD exitCode = 0;
        GetExitCodeProcess(sei.hProcess, &exitCode);
        CloseHandle(sei.hProcess);

        // MSI error code 0 = Success, 3010 = Success (Reboot required), 1602 = Cancelled
        if (exitCode == 0 || exitCode == 3010) {
            logDiagnostic("[InstalledAppManager] Uninstallation completed successfully with exit code " +
                          std::to_string(exitCode));
            return {true, exitCode, "Application uninstalled successfully."};
        } else if (exitCode == 1602) {
            logDiagnostic("[InstalledAppManager] Uninstallation cancelled by user.");
            return {false, exitCode, "Uninstallation cancelled by user in uninstaller wizard."};
        } else {
            logDiagnostic("[InstalledAppManager] Uninstaller exited with non-zero code: " +
                          std::to_string(exitCode));
            return {false, exitCode, "Uninstaller completed with exit code: " + std::to_string(exitCode)};
        }
    } else if (waitRes == WAIT_TIMEOUT) {
        CloseHandle(sei.hProcess);
        logDiagnostic("[InstalledAppManager] Uninstaller is still running in the background.");
        return {true, 0, "Uninstaller is still running in the background."};
    }

    CloseHandle(sei.hProcess);
    return {true, 0, "Uninstaller process launched."};
}

std::vector<InstalledAppInfo> InstalledAppManager::filterByCategory(
    const std::vector<InstalledAppInfo>& apps,
    AppCategory category) {

    if (category == AppCategory::All) {
        return apps;
    }

    std::vector<InstalledAppInfo> filtered;
    for (const auto& a : apps) {
        if (a.category == category) {
            filtered.push_back(a);
        }
    }
    return filtered;
}

std::vector<InstalledAppInfo> InstalledAppManager::filterByName(
    const std::vector<InstalledAppInfo>& apps,
    const std::string& query) {

    if (query.empty()) return apps;
    std::string lowerQuery = toLower(query);

    std::vector<InstalledAppInfo> filtered;
    for (const auto& a : apps) {
        if (toLower(a.name).find(lowerQuery) != std::string::npos ||
            toLower(a.publisher).find(lowerQuery) != std::string::npos) {
            filtered.push_back(a);
        }
    }
    return filtered;
}

void InstalledAppManager::sortApplications(
    std::vector<InstalledAppInfo>& apps,
    SortField field) {

    switch (field) {
        case SortField::NameAsc:
            std::sort(apps.begin(), apps.end(), [](const InstalledAppInfo& a, const InstalledAppInfo& b) {
                return toLower(a.name) < toLower(b.name);
            });
            break;
        case SortField::NameDesc:
            std::sort(apps.begin(), apps.end(), [](const InstalledAppInfo& a, const InstalledAppInfo& b) {
                return toLower(a.name) > toLower(b.name);
            });
            break;
        case SortField::SizeDesc:
            std::sort(apps.begin(), apps.end(), [](const InstalledAppInfo& a, const InstalledAppInfo& b) {
                return a.estimatedSizeBytes > b.estimatedSizeBytes;
            });
            break;
        case SortField::SizeAsc:
            std::sort(apps.begin(), apps.end(), [](const InstalledAppInfo& a, const InstalledAppInfo& b) {
                return a.estimatedSizeBytes < b.estimatedSizeBytes;
            });
            break;
        case SortField::DateDesc:
            std::sort(apps.begin(), apps.end(), [](const InstalledAppInfo& a, const InstalledAppInfo& b) {
                return a.installDateRaw > b.installDateRaw;
            });
            break;
        case SortField::DateAsc:
            std::sort(apps.begin(), apps.end(), [](const InstalledAppInfo& a, const InstalledAppInfo& b) {
                return a.installDateRaw < b.installDateRaw;
            });
            break;
        case SortField::PublisherAsc:
            std::sort(apps.begin(), apps.end(), [](const InstalledAppInfo& a, const InstalledAppInfo& b) {
                return toLower(a.publisher) < toLower(b.publisher);
            });
            break;
    }
}

std::string InstalledAppManager::formatBytes(uint64_t bytes) {
    if (bytes == 0) return "0 B";
    if (bytes < 1024) return std::to_string(bytes) + " B";
    double kb = static_cast<double>(bytes) / 1024.0;
    if (kb < 1024.0) {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(1) << kb << " KB";
        return ss.str();
    }
    double mb = kb / 1024.0;
    if (mb < 1024.0) {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(1) << mb << " MB";
        return ss.str();
    }
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(2) << (mb / 1024.0) << " GB";
    return ss.str();
}

} // namespace pulse


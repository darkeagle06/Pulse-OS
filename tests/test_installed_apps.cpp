#include <iostream>
#include <cassert>
#include <vector>
#include <algorithm>
#include "../src/core/InstalledAppManager.h"

int main() {
    std::cout << "[TestInstalledApps] Starting Installed Application Manager unit tests..." << std::endl;

    // 1. Test formatBytes
    {
        assert(pulse::InstalledAppManager::formatBytes(0) == "0 B");
        assert(pulse::InstalledAppManager::formatBytes(500) == "500 B");
        assert(pulse::InstalledAppManager::formatBytes(1024 * 1024 * 50) == "50.0 MB");
        assert(pulse::InstalledAppManager::formatBytes(1024ULL * 1024 * 1024 * 2) == "2.00 GB");
        std::cout << "  ✓ formatBytes formatting verified." << std::endl;
    }

    // 2. Test Category and Name Filtering
    {
        std::vector<pulse::InstalledAppInfo> apps;

        pulse::InstalledAppInfo app1;
        app1.displayName = "Visual Studio Code";
        app1.publisher = "Microsoft Corporation";
        app1.estimatedSize = 350ULL * 1024 * 1024;
        app1.category = pulse::AppCategory::RecentlyInstalled;
        apps.push_back(app1);

        pulse::InstalledAppInfo app2;
        app2.displayName = "Old Tool";
        app2.publisher = "Legacy Dev";
        app2.estimatedSize = 100ULL * 1024 * 1024;
        app2.category = pulse::AppCategory::PotentiallyUnused;
        apps.push_back(app2);

        pulse::InstalledAppInfo app3;
        app3.displayName = "Heavy Game";
        app3.publisher = "Game Studio";
        app3.estimatedSize = 15ULL * 1024 * 1024 * 1024; // 15 GB
        app3.category = pulse::AppCategory::Large;
        apps.push_back(app3);

        auto unused = pulse::InstalledAppManager::filterByCategory(apps, pulse::AppCategory::PotentiallyUnused);
        assert(unused.size() == 1);
        assert(unused[0].displayName == "Old Tool");

        auto large = pulse::InstalledAppManager::filterByCategory(apps, pulse::AppCategory::Large);
        assert(large.size() == 1);
        assert(large[0].displayName == "Heavy Game");

        auto foundByName = pulse::InstalledAppManager::filterByName(apps, "studio");
        assert(foundByName.size() == 2); // Visual Studio Code, and Heavy Game (publisher Game Studio)

        std::cout << "  ✓ Category and Name filtering verified." << std::endl;
    }

    // 3. Test Sorting
    {
        pulse::InstalledAppInfo a;
        a.displayName = "Beta"; a.name = "Beta"; a.estimatedSize = 500; a.estimatedSizeBytes = 500;
        pulse::InstalledAppInfo b;
        b.displayName = "Alpha"; b.name = "Alpha"; b.estimatedSize = 1000; b.estimatedSizeBytes = 1000;
        pulse::InstalledAppInfo c;
        c.displayName = "Gamma"; c.name = "Gamma"; c.estimatedSize = 200; c.estimatedSizeBytes = 200;

        std::vector<pulse::InstalledAppInfo> apps = {a, b, c};

        pulse::InstalledAppManager::sortApplications(apps, pulse::InstalledAppManager::SortField::NameAsc);
        assert(apps[0].displayName == "Alpha");
        assert(apps[2].displayName == "Gamma");

        pulse::InstalledAppManager::sortApplications(apps, pulse::InstalledAppManager::SortField::SizeDesc);
        assert(apps[0].displayName == "Alpha" && apps[0].estimatedSize == 1000);
        assert(apps[2].displayName == "Gamma" && apps[2].estimatedSize == 200);

        std::cout << "  ✓ Application sorting logic verified." << std::endl;
    }

    // 4. Test Live Windows Registry Scan
    {
        pulse::InstalledAppManager mgr;
        auto liveApps = mgr.scanInstalledApplications({});
        std::cout << "  ✓ Live registry scan discovered " << liveApps.size() << " installed applications." << std::endl;
        assert(!liveApps.empty());

        // Ensure no system components are incorrectly presented as regular apps
        for (const auto& app : liveApps) {
            assert(!app.isSystemComponent);
            assert(!app.displayName.empty());
        }
        std::cout << "  ✓ All discovered applications satisfy validity and safety invariants." << std::endl;
    }

    // 5. Test Safe Uninstaller Handling (Empty or invalid command rejection)
    {
        pulse::InstalledAppManager mgr;
        pulse::InstalledAppInfo invalidApp;
        invalidApp.displayName = "Test Fake App";
        invalidApp.uninstallString = "";
        invalidApp.canUninstall = false;

        auto result = mgr.uninstallApplication(invalidApp, false);
        assert(!result.success);
        assert(result.message.find("No registered uninstaller") != std::string::npos);
        std::cout << "  ✓ Safety guard prevented invalid uninstallation." << std::endl;
    }

    std::cout << "[TestInstalledApps] ALL TESTS PASSED SUCCESSFULLY!" << std::endl;
    return 0;
}


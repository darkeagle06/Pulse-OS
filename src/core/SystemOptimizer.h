#pragma once

#include "Types.h"
#include <vector>
#include <deque>
#include <mutex>
#include <string>
#include <optional>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace pulse {

class SystemOptimizer {
public:
    SystemOptimizer();
    ~SystemOptimizer() = default;

    struct ActionResult {
        bool success{false};
        std::string message;
        std::string actionId;
        std::string previousValue;
        std::string newValue;
        bool canRollback{true};
    };

    // Apply an approved optimization action
    ActionResult applyAction(const OptimizationAction& action);

    // Roll back a previously applied action
    ActionResult rollbackAction(const std::string& actionId);

    // Roll back the most recently applied action
    ActionResult rollbackLastAction();

    // Query action history
    [[nodiscard]] std::vector<OptimizationAction> getAppliedActions() const;
    [[nodiscard]] bool hasRollbackAvailable() const;

    // Supported Documented Windows Actions
    static ActionResult setPowerScheme(const std::string& schemeGuid);
    static ActionResult trimProcessWorkingSets();
    static ActionResult setProcessPriority(uint32_t pid, DWORD priorityClass);
    static ActionResult preventSleep(bool enable);

private:
    static GUID stringToGuid(const std::string& guidStr);
    static std::string guidToString(const GUID& guid);

    mutable std::mutex m_mutex;
    std::deque<OptimizationAction> m_appliedStack;
};

} // namespace pulse


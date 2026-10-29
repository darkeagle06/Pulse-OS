#pragma once

#include "Types.h"
#include <vector>
#include <string>
#include <optional>

namespace pulse {

class IProcessMonitor {
public:
    virtual ~IProcessMonitor() = default;

    // Enumerate active processes and calculate their CPU/memory deltas
    virtual std::vector<ProcessInfo> sampleProcesses() = 0;

    // Retrieve cached list of processes from the most recent sample
    [[nodiscard]] virtual std::vector<ProcessInfo> getLastProcessList() const = 0;

    // Find specific process by PID
    [[nodiscard]] virtual std::optional<ProcessInfo> getProcessByPid(uint32_t pid) const = 0;

    // Foreground application details (PID, Window Title, Process Name)
    struct ForegroundInfo {
        uint32_t pid{0};
        std::string processName;
        std::string windowTitle;
    };
    [[nodiscard]] virtual ForegroundInfo getForegroundProcessInfo() const = 0;
};

} // namespace pulse


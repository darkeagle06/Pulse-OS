#pragma once

#include "Types.h"
#include "IProcessMonitor.h"
#include "ISystemMonitor.h"
#include <memory>
#include <string>

namespace pulse {

class TaskDetector {
public:
    explicit TaskDetector(std::shared_ptr<IProcessMonitor> procMonitor,
                          std::shared_ptr<ISystemMonitor> sysMonitor);
    ~TaskDetector() = default;

    // Analyze current environment and detect the most likely workload
    [[nodiscard]] TaskDetectionResult detect();

    // Check if the machine is running on battery power
    [[nodiscard]] static bool isRunningOnBattery();

private:
    std::shared_ptr<IProcessMonitor> m_procMonitor;
    std::shared_ptr<ISystemMonitor> m_sysMonitor;
};

} // namespace pulse


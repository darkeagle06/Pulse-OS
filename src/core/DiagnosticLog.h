#pragma once

#include <fstream>
#include <iostream>
#include <chrono>
#include <mutex>
#include <string>
#include <sstream>

namespace pulse {

inline void logDiagnostic(const std::string& msg) {
    static std::mutex s_logMutex;
    std::lock_guard<std::mutex> lock(s_logMutex);

    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

    std::ofstream ofs("pulseos_diagnostic.log", std::ios::app);
    if (ofs.is_open()) {
        ofs << "[" << ms << "] " << msg << "\n";
        ofs.flush();
    }
    std::cout << "[" << ms << "] " << msg << std::endl;
}

} // namespace pulse


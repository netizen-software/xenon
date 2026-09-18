#include "core/resources/resource-monitor.hpp"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace xenon {

CpuTicks ResourceMonitor::parseCpuLine(std::string_view line) {
    std::istringstream input{std::string(line)};
    std::string label;
    std::uint64_t user = 0;
    std::uint64_t nice = 0;
    std::uint64_t system = 0;
    std::uint64_t idle = 0;
    std::uint64_t ioWait = 0;
    std::uint64_t irq = 0;
    std::uint64_t softIrq = 0;
    std::uint64_t steal = 0;

    input >> label >> user >> nice >> system >> idle >> ioWait >> irq >> softIrq >> steal;
    if (label != "cpu") {
        return {};
    }

    return {
        .totalTicks = user + nice + system + idle + ioWait + irq + softIrq + steal,
        .idleTicks = idle + ioWait,
    };
}

MemoryStats ResourceMonitor::parseMemoryInfo(std::istream& input) {
    MemoryStats memory{};
    std::string key;
    std::uint64_t value = 0;
    std::string unit;

    while (input >> key >> value >> unit) {
        if (key == "MemTotal:") {
            memory.totalBytes = value * 1024;
        } else if (key == "MemAvailable:") {
            memory.availableBytes = value * 1024;
        }
    }

    return memory;
}

ResourceUsage ResourceMonitor::sample() {
    std::ifstream statInput("/proc/stat");
    std::string cpuLine;
    std::getline(statInput, cpuLine);
    if (!statInput) {
        throw std::runtime_error("Unable to read /proc/stat");
    }

    std::ifstream memoryInput("/proc/meminfo");
    if (!memoryInput) {
        throw std::runtime_error("Unable to read /proc/meminfo");
    }

    const CpuTicks currentCpuTicks = parseCpuLine(cpuLine);
    const MemoryStats memory = parseMemoryInfo(memoryInput);
    double cpuPercent = 0.0;

    if (previousCpuTicks_) {
        const std::uint64_t totalDelta = currentCpuTicks.totalTicks - previousCpuTicks_->totalTicks;
        const std::uint64_t idleDelta = currentCpuTicks.idleTicks - previousCpuTicks_->idleTicks;
        if (totalDelta > 0) {
            cpuPercent = 100.0 * static_cast<double>(totalDelta - idleDelta) /
                         static_cast<double>(totalDelta);
        }
    }

    previousCpuTicks_ = currentCpuTicks;
    return {.cpuPercent = std::clamp(cpuPercent, 0.0, 100.0), .memory = memory};
}

}  // namespace xenon

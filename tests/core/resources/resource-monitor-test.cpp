#include "core/resources/resource-monitor.hpp"

#include <cassert>
#include <sstream>

int main() {
    const xenon::CpuTicks cpu = xenon::ResourceMonitor::parseCpuLine(
        "cpu  470 20 140 700 30 10 15 5 0 0");
    assert(cpu.totalTicks == 1390);
    assert(cpu.idleTicks == 730);

    std::istringstream memoryInput(
        "MemTotal:       16384000 kB\n"
        "MemFree:         1024000 kB\n"
        "MemAvailable:    8192000 kB\n");
    const xenon::MemoryStats memory = xenon::ResourceMonitor::parseMemoryInfo(memoryInput);
    assert(memory.totalBytes == 16777216000ULL);
    assert(memory.availableBytes == 8388608000ULL);

    return 0;
}
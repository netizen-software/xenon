#pragma once

#include <cstdint>
#include <istream>
#include <optional>
#include <string_view>

namespace xenon {

struct CpuTicks {
    std::uint64_t totalTicks;
    std::uint64_t idleTicks;
};

struct MemoryStats {
    std::uint64_t totalBytes;
    std::uint64_t availableBytes;
};

struct ResourceUsage {
    double cpuPercent;
    MemoryStats memory;
};

class ResourceMonitor {
   public:
    ResourceUsage sample();

    static CpuTicks parseCpuLine(std::string_view line);
    static MemoryStats parseMemoryInfo(std::istream& input);

   private:
    std::optional<CpuTicks> previousCpuTicks_;
};

}  // namespace xenon

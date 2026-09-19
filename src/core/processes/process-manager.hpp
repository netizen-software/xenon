#pragma once

#include <cstdint>
#include <istream>
#include <string>
#include <vector>

namespace xenon {

struct ProcessInfo {
    std::uint32_t pid;
    std::string name;
    std::string state;
    std::uint64_t memoryBytes;
};

class ProcessManager {
   public:
    std::vector<ProcessInfo> listProcesses() const;
    bool terminateProcess(std::uint32_t pid) const;

    static ProcessInfo parseStatus(std::uint32_t pid, std::istream& input);
};

}  // namespace xenon

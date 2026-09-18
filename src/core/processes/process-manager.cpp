#include "core/processes/process-manager.hpp"

#include <unistd.h>

#include <algorithm>
#include <csignal>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string_view>

namespace {

std::string valueAfterColon(std::string_view line) {
    const std::size_t colonPosition = line.find(':');
    if (colonPosition == std::string_view::npos) {
        return {};
    }

    const std::size_t valuePosition = line.find_first_not_of(" \t", colonPosition + 1);
    return valuePosition == std::string_view::npos ? std::string{}
                                                   : std::string(line.substr(valuePosition));
}

}  // namespace

namespace xenon {

ProcessInfo ProcessManager::parseStatus(std::uint32_t pid, std::istream& input) {
    ProcessInfo process{.pid = pid, .name = {}, .state = {}, .memoryBytes = 0};
    std::string line;

    while (std::getline(input, line)) {
        if (line.starts_with("Name:")) {
            process.name = valueAfterColon(line);
        } else if (line.starts_with("State:")) {
            process.state = valueAfterColon(line);
        } else if (line.starts_with("VmRSS:")) {
            std::istringstream memoryInput(valueAfterColon(line));
            std::uint64_t memoryKilobytes = 0;
            memoryInput >> memoryKilobytes;
            process.memoryBytes = memoryKilobytes * 1024;
        }
    }

    return process;
}

std::vector<ProcessInfo> ProcessManager::listProcesses() const {
    std::vector<ProcessInfo> processes;

    for (const auto& entry : std::filesystem::directory_iterator("/proc")) {
        if (!entry.is_directory()) {
            continue;
        }

        const std::string directoryName = entry.path().filename().string();
        if (!std::all_of(directoryName.begin(), directoryName.end(), ::isdigit)) {
            continue;
        }

        const auto pid = static_cast<std::uint32_t>(std::stoul(directoryName));
        std::ifstream statusInput(entry.path() / "status");
        if (!statusInput) {
            continue;
        }

        ProcessInfo process = parseStatus(pid, statusInput);
        if (!process.name.empty()) {
            processes.push_back(std::move(process));
        }
    }

    std::ranges::sort(processes, {}, &ProcessInfo::pid);
    return processes;
}

bool ProcessManager::terminateProcess(std::uint32_t pid) const {
    return kill(static_cast<pid_t>(pid), SIGTERM) == 0;
}

}  // namespace xenon

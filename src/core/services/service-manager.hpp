#pragma once

#include <span>
#include <string>
#include <utility>
#include <vector>

namespace xenon {

struct ServiceInfo {
    std::string unitName;
    std::string enablement;
    std::string runtimeState;
};

class ServiceManager {
   public:
    std::vector<ServiceInfo> listServices() const;

    static std::vector<ServiceInfo> filterServiceUnits(
        std::span<const std::pair<std::string, std::string>> unitFiles);
    static void addRuntimeStates(std::span<ServiceInfo> services,
                                 std::span<const std::pair<std::string, std::string>> unitStates);
};

}  // namespace xenon
